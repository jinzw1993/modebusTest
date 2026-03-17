/**
 * @file main.c
 * @brief Modbus RTU从站示例程序
 * @author Claude
 * @date 2026-03-17
 *
 * @note 这是一个示例主程序，展示如何使用Modbus从站协议栈
 */

#include <stdio.h>
#include <string.h>
#include "app/modbus_slave.h"
#include "app/modbus_data.h"
#include "app/modbus_config.h"
#include "hal/hal_uart.h"
#include "hal/hal_timer.h"

/* ============================================================================
 * 移植层声明
 * ============================================================================ */

#if defined(AT32F423) || defined(AT32F423K8U7_4)
extern void port_uart_at32_register(void);
extern void port_timer_at32_register(void);
#define port_uart_register  port_uart_at32_register
#define port_timer_register port_timer_at32_register
#else
extern void port_uart_stm32_register(void);
extern void port_timer_stm32_register(void);
#define port_uart_register  port_uart_stm32_register
#define port_timer_register port_timer_stm32_register
#endif

/* ============================================================================
 * 应用层数据区（静态分配）
 * ============================================================================ */

/* 线圈缓冲区（1024个线圈 = 128字节） */
static uint8_t s_coils_buffer[128];

/* 离散输入缓冲区（1024个离散输入 = 128字节） */
static uint8_t s_discrete_inputs_buffer[128];

/* 保持寄存器缓冲区（256个寄存器 = 512字节） */
static uint16_t s_holding_registers_buffer[256];

/* 输入寄存器缓冲区（256个寄存器 = 512字节） */
static uint16_t s_input_registers_buffer[256];

/* ============================================================================
 * 应用层回调函数示例
 * ============================================================================ */

/**
 * @brief 保持寄存器写入回调函数
 * @param addr 寄存器地址
 * @param value 写入的值
 */
static void on_holding_reg_written(uint16_t addr, uint16_t value)
{
    printf("保持寄存器[%d]被写入: 0x%04X (%d)\n", addr, value, value);

    /* 根据地址执行特定操作 */
    switch (addr) {
        case 0:
            /* 控制寄存器 */
            if (value & 0x0001) {
                printf("  -> 启动命令\n");
            }
            if (value & 0x0002) {
                printf("  -> 停止命令\n");
            }
            break;

        case 1:
            /* 设定值寄存器 */
            printf("  -> 新设定值: %d\n", value);
            break;

        case 2:
            /* 模式选择寄存器 */
            printf("  -> 模式: %d\n", value);
            break;

        default:
            break;
    }
}

/**
 * @brief 输入寄存器读取回调函数
 * @param addr 寄存器地址
 * @return 寄存器值
 */
static uint16_t on_input_reg_read(uint16_t addr)
{
    uint16_t value = 0;

    /* 根据地址返回实际数据（通常来自传感器或ADC） */
    switch (addr) {
        case 0:
            /* 温度值（示例：25.6°C = 256） */
            value = 256;
            break;

        case 1:
            /* 湿度值（示例：65%） */
            value = 650;
            break;

        case 2:
            /* 压力值（示例：101.3kPa = 1013） */
            value = 1013;
            break;

        case 3:
            /* 状态字 */
            value = 0x00FF;  /* 所有状态正常 */
            break;

        default:
            /* 返回缓冲区中的值 */
            value = s_input_registers_buffer[addr];
            break;
    }

    return value;
}

/**
 * @brief 线圈写入回调函数
 * @param addr 线圈地址
 * @param value 写入的值
 */
static void on_coil_written(uint16_t addr, uint8_t value)
{
    printf("线圈[%d]被写入: %s\n", addr, value ? "ON" : "OFF");

    /* 根据地址控制实际输出（如继电器） */
    switch (addr) {
        case 0:
            /* 控制继电器1 */
            printf("  -> 继电器1: %s\n", value ? "吸合" : "断开");
            break;

        case 1:
            /* 控制继电器2 */
            printf("  -> 继电器2: %s\n", value ? "吸合" : "断开");
            break;

        default:
            break;
    }
}

/**
 * @brief 离散输入读取回调函数
 * @param addr 离散输入地址
 * @return 输入值
 */
static uint8_t on_discrete_input_read(uint16_t addr)
{
    uint8_t value = 0;

    /* 根据地址返回实际输入状态（通常来自开关或传感器） */
    switch (addr) {
        case 0:
            /* 限位开关1 */
            value = 1;  /* 未触发 */
            break;

        case 1:
            /* 限位开关2 */
            value = 0;  /* 已触发 */
            break;

        case 2:
            /* 急停按钮 */
            value = 1;  /* 正常 */
            break;

        default:
            /* 返回缓冲区中的值 */
            {
                uint8_t byte_idx = addr / 8;
                uint8_t bit_idx = addr % 8;
                if (byte_idx < 128) {
                    value = (s_discrete_inputs_buffer[byte_idx] >> bit_idx) & 0x01;
                }
            }
            break;
    }

    return value;
}

/**
 * @brief 错误回调函数
 * @param error 错误码
 */
static void on_error(uint8_t error)
{
    printf("Modbus错误: %d\n", error);
}

/* ============================================================================
 * 初始化函数
 * ============================================================================ */

/**
 * @brief 系统初始化
 */
static void system_init(void)
{
    printf("===========================================\n");
    printf("  Modbus RTU从站示例程序\n");
    printf("  从站地址: %d\n", MB_SLAVE_ID_DEFAULT);
    printf("  波特率: %d, 偶校验\n", MB_UART_BAUDRATE_DEFAULT);
    printf("===========================================\n");

    /* 注册HAL接口 */
    port_uart_register();
    port_timer_register();

    /* 配置数据区 */
    mb_data_config_t data_cfg = {
        .coils = s_coils_buffer,
        .coils_size = 1024,
        .read_coil_cb = NULL,           /* 使用缓冲区读取 */
        .write_coil_cb = on_coil_written,

        .discrete_inputs = s_discrete_inputs_buffer,
        .discrete_inputs_size = 1024,
        .read_discrete_input_cb = on_discrete_input_read,

        .holding_registers = s_holding_registers_buffer,
        .holding_registers_size = 256,
        .read_holding_reg_cb = NULL,    /* 使用缓冲区读取 */
        .write_holding_reg_cb = on_holding_reg_written,

        .input_registers = s_input_registers_buffer,
        .input_registers_size = 256,
        .read_input_reg_cb = on_input_reg_read
    };

    /* 初始化数据区 */
    modbus_data_init(&data_cfg);

    /* 初始化Modbus从站 */
    modbus_slave_init(MB_SLAVE_ID_DEFAULT);

    /* 设置回调函数 */
    modbus_slave_set_error_callback(on_error);

    /* 初始化串口 */
    hal_uart_init(MB_UART_BAUDRATE_DEFAULT, HAL_UART_PARITY_EVEN);

    /* 初始化T3.5定时器 */
    hal_timer_init(MB_T35_TIMEOUT_MS);

    printf("初始化完成，等待Modbus请求...\n");
}

/* ============================================================================
 * 主程序
 * ============================================================================ */

/**
 * @brief 主函数
 */
int main(void)
{
    /* 系统初始化 */
    system_init();

    /* 主循环 */
    while (1) {
        /* 状态机轮询（如果使用轮询模式） */
        modbus_slave_poll();

        /* 应用层任务 */
        /* ... */

        /* 简单延时（实际应使用RTOS或定时器） */
        /* HAL_Delay(1); */
    }

    return 0;
}

/* ============================================================================
 * 调试和测试函数
 * ============================================================================ */

#if MB_DEBUG_ENABLE

/**
 * @brief 打印Modbus统计信息
 */
void print_stats(void)
{
#if MB_ENABLE_STATS
    mb_stats_t stats;
    modbus_slave_get_stats(&stats);

    printf("\n--- Modbus统计 ---\n");
    printf("接收帧数: %lu\n", stats.rx_count);
    printf("发送帧数: %lu\n", stats.tx_count);
    printf("错误计数: %lu\n", stats.error_count);
    printf("CRC错误: %lu\n", stats.crc_error_count);
    printf("广播帧数: %lu\n", stats.broadcast_count);
    printf("------------------\n\n");
#endif
}

/**
 * @brief 模拟接收一个Modbus请求帧（用于测试）
 * @param frame 帧数据
 * @param len 帧长度
 */
void simulate_request(const uint8_t *frame, uint16_t len)
{
    uint16_t i;

    printf("模拟接收帧: ");
    for (i = 0; i < len; i++) {
        printf("%02X ", frame[i]);
        modbus_slave_rx_byte_isr(frame[i]);
    }
    printf("\n");

    /* 模拟T3.5超时 */
    modbus_slave_t35_expired_isr();
}

/**
 * @brief 测试功能
 */
void run_self_test(void)
{
    /* 读保持寄存器请求示例：从站地址=1, 功能码=03, 起始地址=0, 数量=10 */
    /* 01 03 00 00 00 0A C5 CD */
    uint8_t read_regs_request[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD };

    printf("\n=== 自测试开始 ===\n");

    /* 预设一些保持寄存器值 */
    s_holding_registers_buffer[0] = 0x1234;
    s_holding_registers_buffer[1] = 0x5678;
    s_holding_registers_buffer[2] = 0x9ABC;

    /* 模拟接收读保持寄存器请求 */
    simulate_request(read_regs_request, sizeof(read_regs_request));

    printf("=== 自测试完成 ===\n\n");
}

#endif /* MB_DEBUG_ENABLE */
