/**
 * @file modbus_slave.c
 * @brief Modbus从站状态机实现
 * @author Claude
 * @date 2026-03-17
 */

#include "modbus_slave.h"
#include "modbus_rtu.h"
#include "modbus_pdu.h"
#include "modbus_crc.h"
#include "modbus_config.h"
#include "../hal/hal_uart.h"
#include "../hal/hal_timer.h"
#include <string.h>

/* ============================================================================
 * 内部类型定义
 * ============================================================================ */

/* 缓冲区结构 */
typedef struct {
    uint8_t buffer[MB_BUFFER_SIZE];
    uint16_t length;
} mb_buffer_t;

/* 状态转换动作函数类型 */
typedef void (*mb_action_func_t)(void);

/* 状态转换表项 */
typedef struct {
    mb_state_t      current_state;  /* 当前状态 */
    mb_event_t      event;          /* 触发事件 */
    mb_state_t      next_state;     /* 下一状态 */
    mb_action_func_t action;        /* 执行动作 */
} mb_state_transition_t;

/* ============================================================================
 * 全局实例
 * ============================================================================ */

static struct {
    /* 状态机 */
    mb_state_t state;
    mb_error_t last_error;

    /* 从站地址 */
    uint8_t slave_id;

    /* 收发缓冲区 */
    mb_buffer_t rx_buf;
    mb_buffer_t tx_buf;

    /* 当前接收的字节 */
    uint8_t rx_byte;

    /* 回调函数 */
    mb_callback_t on_rx_callback;
    mb_callback_t on_tx_callback;
    mb_error_cb_t on_error_callback;

#if MB_ENABLE_STATS
    /* 统计信息 */
    mb_stats_t stats;
#else
    uint32_t reserved;  /* 占位 */
#endif

    /* 初始化标志 */
    uint8_t initialized;
} g_modbus = {0};

/* ============================================================================
 * 状态转换动作函数声明
 * ============================================================================ */

static void action_start_rx(void);
static void action_rx_byte(void);
static void action_process_frame(void);
static void action_start_tx(void);
static void action_tx_complete(void);
static void action_handle_error(void);
static void action_reset(void);

/* ============================================================================
 * 状态转换表
 * ============================================================================ */

static const mb_state_transition_t s_state_table[] = {
    /* 当前状态                事件                      下一状态               动作 */
    { MB_STATE_IDLE,          MB_EVENT_BYTE_RECEIVED,   MB_STATE_RECEIVING,   action_start_rx   },
    { MB_STATE_IDLE,          MB_EVENT_RESET,           MB_STATE_IDLE,        action_reset      },

    { MB_STATE_RECEIVING,     MB_EVENT_BYTE_RECEIVED,   MB_STATE_RECEIVING,   action_rx_byte    },
    { MB_STATE_RECEIVING,     MB_EVENT_T35_EXPIRED,     MB_STATE_PROCESSING,  action_process_frame },
    { MB_STATE_RECEIVING,     MB_EVENT_ERROR_DETECTED,  MB_STATE_ERROR,       action_handle_error },

    { MB_STATE_PROCESSING,    MB_EVENT_PROCESS_COMPLETE,MB_STATE_RESPONDING,  action_start_tx   },
    { MB_STATE_PROCESSING,    MB_EVENT_ERROR_DETECTED,  MB_STATE_IDLE,        action_reset      },

    { MB_STATE_RESPONDING,    MB_EVENT_TX_COMPLETE,     MB_STATE_IDLE,        action_tx_complete },
    { MB_STATE_RESPONDING,    MB_EVENT_ERROR_DETECTED,  MB_STATE_ERROR,       action_handle_error },

    { MB_STATE_ERROR,         MB_EVENT_RESET,           MB_STATE_IDLE,        action_reset      },

    /* 表结束标记 */
    { MB_STATE_IDLE,          MB_EVENT_NONE,            MB_STATE_IDLE,        NULL              }
};

/* ============================================================================
 * 状态机引擎
 * ============================================================================ */

/**
 * @brief 处理状态机事件
 */
static void state_machine_process_event(mb_event_t event)
{
    const mb_state_transition_t *trans;
    uint16_t table_size = sizeof(s_state_table) / sizeof(s_state_table[0]);
    uint16_t i;

    /* 遍历状态转换表 */
    for (i = 0; i < table_size; i++) {
        trans = &s_state_table[i];

        /* 检查表结束 */
        if (trans->action == NULL && trans->event == MB_EVENT_NONE) {
            break;
        }

        /* 匹配当前状态和事件 */
        if (trans->current_state == g_modbus.state && trans->event == event) {
            /* 执行动作 */
            if (trans->action != NULL) {
                trans->action();
            }

            /* 状态转换 */
            g_modbus.state = trans->next_state;
            return;
        }
    }

    /* 未找到匹配的转换，忽略事件 */
}

/* ============================================================================
 * 状态转换动作函数实现
 * ============================================================================ */

/**
 * @brief 开始接收动作
 */
static void action_start_rx(void)
{
    /* 清空接收缓冲区 */
    g_modbus.rx_buf.length = 0;
    g_modbus.rx_buf.buffer[0] = g_modbus.rx_byte;
    g_modbus.rx_buf.length = 1;

    /* 启动T3.5定时器 */
    hal_timer_reset();
    hal_timer_start();

    /* 清除错误 */
    g_modbus.last_error = MB_ERROR_NONE;

    MB_DEBUG_PRINT("开始接收, 首字节: 0x%02X", g_modbus.rx_byte);
}

/**
 * @brief 接收字节动作
 */
static void action_rx_byte(void)
{
    /* 检查缓冲区是否已满 */
    if (g_modbus.rx_buf.length >= MB_BUFFER_SIZE) {
        g_modbus.last_error = MB_ERROR_BUFFER_OVERFLOW;
        state_machine_process_event(MB_EVENT_ERROR_DETECTED);
        return;
    }

    /* 存储字节 */
    g_modbus.rx_buf.buffer[g_modbus.rx_buf.length++] = g_modbus.rx_byte;

    /* 重置T3.5定时器 */
    hal_timer_reset();
}

/**
 * @brief 处理帧动作
 */
static void action_process_frame(void)
{
    mb_rtu_frame_t frame;
    mb_pdu_result_t pdu_result;
    int ret;

    /* 停止定时器 */
    hal_timer_stop();

    MB_DEBUG_PRINT("帧接收完成, 长度: %d", g_modbus.rx_buf.length);

#if MB_ENABLE_STATS
    g_modbus.stats.rx_count++;
#endif

    /* 验证CRC */
    if (modbus_crc_verify(g_modbus.rx_buf.buffer, g_modbus.rx_buf.length) != 0) {
        MB_DEBUG_PRINT("CRC校验失败");
#if MB_ENABLE_STATS
        g_modbus.stats.crc_error_count++;
        g_modbus.stats.error_count++;
#endif
        g_modbus.last_error = MB_ERROR_CRC;
        state_machine_process_event(MB_EVENT_RESET);
        return;
    }

    /* 解析RTU帧 */
    ret = modbus_rtu_parse(g_modbus.rx_buf.buffer, g_modbus.rx_buf.length, &frame);
    if (ret != 0) {
        MB_DEBUG_PRINT("帧解析失败");
#if MB_ENABLE_STATS
        g_modbus.stats.error_count++;
#endif
        state_machine_process_event(MB_EVENT_RESET);
        return;
    }

    /* 检查从站地址 */
    if (frame.slave_addr != g_modbus.slave_id) {
#if MB_SUPPORT_BROADCAST
        if (frame.slave_addr == MB_SLAVE_ADDR_BROADCAST) {
            /* 广播帧，处理但不响应 */
#if MB_ENABLE_STATS
            g_modbus.stats.broadcast_count++;
#endif
            MB_DEBUG_PRINT("收到广播帧");
        } else {
            /* 不是发给本站的 */
            MB_DEBUG_PRINT("地址不匹配: %d != %d", frame.slave_addr, g_modbus.slave_id);
            state_machine_process_event(MB_EVENT_RESET);
            return;
        }
#else
        /* 不是发给本站的 */
        MB_DEBUG_PRINT("地址不匹配: %d != %d", frame.slave_addr, g_modbus.slave_id);
        state_machine_process_event(MB_EVENT_RESET);
        return;
#endif
    }

    MB_DEBUG_PRINT("处理功能码: 0x%02X", frame.function_code);

    /* 调用接收回调 */
    if (g_modbus.on_rx_callback) {
        g_modbus.on_rx_callback();
    }

    /* 处理PDU */
    pdu_result = modbus_pdu_process(
        frame.function_code,
        frame.data,
        frame.data_len,
        &g_modbus.tx_buf.buffer[2],  /* 跳过地址和功能码 */
        MB_BUFFER_SIZE - 4           /* 预留地址、功能码和CRC */
    );

    /* 构建响应帧 */
    if (pdu_result.exception == MB_EX_NONE) {
        /* 正常响应 */
        g_modbus.tx_buf.buffer[0] = g_modbus.slave_id;
        g_modbus.tx_buf.buffer[1] = frame.function_code;
        g_modbus.tx_buf.length = 2 + pdu_result.response_len;

        /* 添加CRC */
        modbus_crc_append(g_modbus.tx_buf.buffer, g_modbus.tx_buf.length);
        g_modbus.tx_buf.length += 2;

#if MB_SUPPORT_BROADCAST
        /* 广播帧不响应 */
        if (frame.slave_addr == MB_SLAVE_ADDR_BROADCAST) {
            state_machine_process_event(MB_EVENT_RESET);
            return;
        }
#endif
    } else {
        /* 异常响应 */
#if MB_SUPPORT_BROADCAST
        /* 广播帧不响应 */
        if (frame.slave_addr == MB_SLAVE_ADDR_BROADCAST) {
            state_machine_process_event(MB_EVENT_RESET);
            return;
        }
#endif

        g_modbus.tx_buf.length = modbus_rtu_build_exception(
            g_modbus.slave_id,
            frame.function_code,
            pdu_result.exception,
            g_modbus.tx_buf.buffer
        );

        MB_DEBUG_PRINT("异常响应: 0x%02X", pdu_result.exception);
    }

    state_machine_process_event(MB_EVENT_PROCESS_COMPLETE);
}

/**
 * @brief 开始发送动作
 */
static void action_start_tx(void)
{
    MB_DEBUG_PRINT("开始发送, 长度: %d", g_modbus.tx_buf.length);

    /* 发送响应帧 */
    hal_uart_send(g_modbus.tx_buf.buffer, g_modbus.tx_buf.length);

#if MB_ENABLE_STATS
    g_modbus.stats.tx_count++;
#endif
}

/**
 * @brief 发送完成动作
 */
static void action_tx_complete(void)
{
    MB_DEBUG_PRINT("发送完成");

    /* 调用发送回调 */
    if (g_modbus.on_tx_callback) {
        g_modbus.on_tx_callback();
    }

    /* 清空缓冲区 */
    g_modbus.rx_buf.length = 0;
    g_modbus.tx_buf.length = 0;
}

/**
 * @brief 错误处理动作
 */
static void action_handle_error(void)
{
    MB_DEBUG_PRINT("错误: %d", g_modbus.last_error);

#if MB_ENABLE_STATS
    g_modbus.stats.error_count++;
#endif

    /* 调用错误回调 */
    if (g_modbus.on_error_callback) {
        g_modbus.on_error_callback(g_modbus.last_error);
    }

    /* 复位 */
    action_reset();
}

/**
 * @brief 复位动作
 */
static void action_reset(void)
{
    hal_timer_stop();
    g_modbus.rx_buf.length = 0;
    g_modbus.tx_buf.length = 0;
    g_modbus.last_error = MB_ERROR_NONE;
    g_modbus.state = MB_STATE_IDLE;

    MB_DEBUG_PRINT("状态机复位");
}

/* ============================================================================
 * 中断回调函数
 * ============================================================================ */

/**
 * @brief 串口接收中断回调
 */
static void uart_rx_callback(uint8_t byte)
{
    g_modbus.rx_byte = byte;

    /* 发送字节接收事件 */
    if (g_modbus.state == MB_STATE_IDLE) {
        state_machine_process_event(MB_EVENT_BYTE_RECEIVED);
    } else if (g_modbus.state == MB_STATE_RECEIVING) {
        state_machine_process_event(MB_EVENT_BYTE_RECEIVED);
    }
}

/**
 * @brief 定时器超时回调
 */
static void timer_timeout_callback(void)
{
    /* 发送T3.5超时事件 */
    if (g_modbus.state == MB_STATE_RECEIVING) {
        state_machine_process_event(MB_EVENT_T35_EXPIRED);
    }
}

/**
 * @brief 串口发送完成回调
 */
static void uart_tx_complete_callback(void)
{
    /* 发送发送完成事件 */
    if (g_modbus.state == MB_STATE_RESPONDING) {
        state_machine_process_event(MB_EVENT_TX_COMPLETE);
    }
}

/* ============================================================================
 * API实现
 * ============================================================================ */

int modbus_slave_init(uint8_t slave_id)
{
    /* 检查从站地址 */
    if (slave_id < MB_SLAVE_ADDR_MIN || slave_id > MB_SLAVE_ADDR_MAX) {
        return -1;
    }

    /* 初始化全局结构 */
    memset(&g_modbus, 0, sizeof(g_modbus));
    g_modbus.slave_id = slave_id;
    g_modbus.state = MB_STATE_IDLE;
    g_modbus.initialized = 1;

    /* 设置HAL回调 */
    hal_uart_set_rx_callback(uart_rx_callback);
    hal_uart_set_tx_complete_callback(uart_tx_complete_callback);
    hal_timer_set_callback(timer_timeout_callback);

    MB_DEBUG_PRINT("Modbus从站初始化完成, 地址: %d", slave_id);

    return 0;
}

void modbus_slave_deinit(void)
{
    hal_timer_stop();
    hal_uart_set_rx_callback(NULL);
    hal_uart_set_tx_complete_callback(NULL);
    hal_timer_set_callback(NULL);

    memset(&g_modbus, 0, sizeof(g_modbus));

    MB_DEBUG_PRINT("Modbus从站反初始化");
}

int modbus_slave_set_data_config(const mb_data_config_t *config)
{
    return modbus_data_init(config);
}

void modbus_slave_set_address(uint8_t slave_id)
{
    if (slave_id >= MB_SLAVE_ADDR_MIN && slave_id <= MB_SLAVE_ADDR_MAX) {
        g_modbus.slave_id = slave_id;
        MB_DEBUG_PRINT("从站地址修改为: %d", slave_id);
    }
}

uint8_t modbus_slave_get_address(void)
{
    return g_modbus.slave_id;
}

void modbus_slave_poll(void)
{
    /* 主循环轮询，目前状态机在中断中驱动 */
    /* 如果需要轮询模式，可以在此处理 */
}

mb_state_t modbus_slave_get_state(void)
{
    return g_modbus.state;
}

void modbus_slave_rx_byte_isr(uint8_t byte)
{
    uart_rx_callback(byte);
}

void modbus_slave_t35_expired_isr(void)
{
    timer_timeout_callback();
}

void modbus_slave_tx_complete_isr(void)
{
    uart_tx_complete_callback();
}

void modbus_slave_set_rx_callback(mb_callback_t callback)
{
    g_modbus.on_rx_callback = callback;
}

void modbus_slave_set_tx_callback(mb_callback_t callback)
{
    g_modbus.on_tx_callback = callback;
}

void modbus_slave_set_error_callback(mb_error_cb_t callback)
{
    g_modbus.on_error_callback = callback;
}

#if MB_ENABLE_STATS
void modbus_slave_get_stats(mb_stats_t *stats)
{
#if MB_ENABLE_STATS
    if (stats != NULL) {
        *stats = g_modbus.stats;
    }
#else
    (void)stats;
#endif
}

void modbus_slave_reset_stats(void)
{
#if MB_ENABLE_STATS
    memset(&g_modbus.stats, 0, sizeof(g_modbus.stats));
#endif
}
