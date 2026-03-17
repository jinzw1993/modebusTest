/**
 * @file modbus_slave.h
 * @brief Modbus从站状态机接口
 * @author Claude
 * @date 2026-03-17
 */

#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <stdint.h>
#include <stdbool.h>
#include "modbus_types.h"
#include "modbus_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 统计信息结构
 * ============================================================================ */

typedef struct {
    uint32_t rx_count;          /* 接收帧计数 */
    uint32_t tx_count;          /* 发送帧计数 */
    uint32_t error_count;       /* 错误计数 */
    uint32_t crc_error_count;   /* CRC错误计数 */
    uint32_t broadcast_count;   /* 广播帧计数 */
} mb_stats_t;

/* ============================================================================
 * 初始化和配置
 * ============================================================================ */

/**
 * @brief 初始化Modbus从站
 * @param slave_id 从站地址 (1-247)
 * @return 0:成功, <0:失败
 */
int modbus_slave_init(uint8_t slave_id);

/**
 * @brief 反初始化Modbus从站
 */
void modbus_slave_deinit(void);

/**
 * @brief 设置数据区配置
 * @param config 数据区配置
 * @return 0:成功, <0:失败
 */
int modbus_slave_set_data_config(const mb_data_config_t *config);

/**
 * @brief 设置从站地址
 * @param slave_id 新的从站地址 (1-247)
 */
void modbus_slave_set_address(uint8_t slave_id);

/**
 * @brief 获取当前从站地址
 * @return 从站地址
 */
uint8_t modbus_slave_get_address(void);

/* ============================================================================
 * 状态机操作
 * ============================================================================ */

/**
 * @brief 状态机轮询（主循环调用）
 * @note 应在主循环中周期调用
 */
void modbus_slave_poll(void);

/**
 * @brief 获取当前状态
 * @return 状态值
 */
mb_state_t modbus_slave_get_state(void);

/* ============================================================================
 * 中断回调（ISR中调用）
 * ============================================================================ */

/**
 * @brief 处理接收到的字节（ISR调用）
 * @param byte 接收到的字节
 * @note 此函数应在串口接收中断中调用
 */
void modbus_slave_rx_byte_isr(uint8_t byte);

/**
 * @brief 处理T3.5超时（ISR调用）
 * @note 此函数应在定时器超时中断中调用
 */
void modbus_slave_t35_expired_isr(void);

/**
 * @brief 处理发送完成（ISR调用）
 * @note 此函数应在串口发送完成中断中调用
 */
void modbus_slave_tx_complete_isr(void);

/* ============================================================================
 * 事件回调设置
 * ============================================================================ */

/**
 * @brief 设置请求接收回调
 * @param callback 回调函数
 */
void modbus_slave_set_rx_callback(mb_callback_t callback);

/**
 * @brief 设置响应发送回调
 * @param callback 回调函数
 */
void modbus_slave_set_tx_callback(mb_callback_t callback);

/**
 * @brief 设置错误回调
 * @param callback 回调函数
 */
void modbus_slave_set_error_callback(mb_error_cb_t callback);

/* ============================================================================
 * 统计信息
 * ============================================================================ */

/**
 * @brief 获取统计信息
 * @param stats 统计信息结构
 */
void modbus_slave_get_stats(mb_stats_t *stats);

/**
 * @brief 重置统计信息
 */
void modbus_slave_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_SLAVE_H */
