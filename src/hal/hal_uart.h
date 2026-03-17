/**
 * @file hal_uart.h
 * @brief 串口硬件抽象层接口
 * @author Claude
 * @date 2026-03-17
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 校验类型定义
 */
typedef enum {
    HAL_UART_PARITY_NONE = 0,   /* 无校验 */
    HAL_UART_PARITY_ODD  = 1,   /* 奇校验 */
    HAL_UART_PARITY_EVEN = 2    /* 偶校验 */
} hal_uart_parity_t;

/**
 * @brief 串口操作接口结构
 */
typedef struct {
    /**
     * @brief 初始化串口
     * @param baudrate 波特率
     * @param parity 校验方式
     * @return 0:成功, <0:失败
     */
    int (*init)(uint32_t baudrate, hal_uart_parity_t parity);

    /**
     * @brief 反初始化串口
     */
    void (*deinit)(void);

    /**
     * @brief 发送数据（异步）
     * @param data 数据指针
     * @param len 数据长度
     * @return 0:成功, <0:失败
     */
    int (*send)(const uint8_t *data, uint16_t len);

    /**
     * @brief 设置接收字节回调函数
     * @param callback 回调函数指针
     */
    void (*set_rx_callback)(void (*callback)(uint8_t byte));

    /**
     * @brief 设置发送完成回调函数
     * @param callback 回调函数指针
     */
    void (*set_tx_complete_callback)(void (*callback)(void));
} hal_uart_t;

/* ============================================================================
 * HAL接口函数
 * ============================================================================ */

/**
 * @brief 注册串口HAL接口
 * @param hal 串口操作接口结构指针
 * @return 0:成功, <0:失败
 */
int hal_uart_register(const hal_uart_t *hal);

/**
 * @brief 初始化串口
 * @param baudrate 波特率
 * @param parity 校验方式
 * @return 0:成功, <0:失败
 */
int hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity);

/**
 * @brief 反初始化串口
 */
void hal_uart_deinit(void);

/**
 * @brief 发送数据（异步）
 * @param data 数据指针
 * @param len 数据长度
 * @return 0:成功, <0:失败
 */
int hal_uart_send(const uint8_t *data, uint16_t len);

/**
 * @brief 设置接收字节回调函数
 * @param callback 回调函数指针
 */
void hal_uart_set_rx_callback(void (*callback)(uint8_t byte));

/**
 * @brief 设置发送完成回调函数
 * @param callback 回调函数指针
 */
void hal_uart_set_tx_complete_callback(void (*callback)(void));

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */
