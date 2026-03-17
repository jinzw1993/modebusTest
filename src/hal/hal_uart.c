/**
 * @file hal_uart.c
 * @brief 串口硬件抽象层实现
 * @author Claude
 * @date 2026-03-17
 */

#include "hal_uart.h"
#include <stddef.h>

/* 当前注册的HAL接口 */
static const hal_uart_t *s_hal_uart = NULL;

/**
 * @brief 注册串口HAL接口
 */
int hal_uart_register(const hal_uart_t *hal)
{
    if (hal == NULL) {
        return -1;
    }

    s_hal_uart = hal;
    return 0;
}

/**
 * @brief 初始化串口
 */
int hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity)
{
    if (s_hal_uart == NULL || s_hal_uart->init == NULL) {
        return -1;
    }

    return s_hal_uart->init(baudrate, parity);
}

/**
 * @brief 反初始化串口
 */
void hal_uart_deinit(void)
{
    if (s_hal_uart != NULL && s_hal_uart->deinit != NULL) {
        s_hal_uart->deinit();
    }
}

/**
 * @brief 发送数据
 */
int hal_uart_send(const uint8_t *data, uint16_t len)
{
    if (s_hal_uart == NULL || s_hal_uart->send == NULL) {
        return -1;
    }

    return s_hal_uart->send(data, len);
}

/**
 * @brief 设置接收字节回调函数
 */
void hal_uart_set_rx_callback(void (*callback)(uint8_t byte))
{
    if (s_hal_uart != NULL && s_hal_uart->set_rx_callback != NULL) {
        s_hal_uart->set_rx_callback(callback);
    }
}

/**
 * @brief 设置发送完成回调函数
 */
void hal_uart_set_tx_complete_callback(void (*callback)(void))
{
    if (s_hal_uart != NULL && s_hal_uart->set_tx_complete_callback != NULL) {
        s_hal_uart->set_tx_complete_callback(callback);
    }
}
