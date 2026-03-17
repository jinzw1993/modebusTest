/**
 * @file port_uart_stm32.c
 * @brief STM32串口移植实现
 * @author Claude
 * @date 2026-03-17
 *
 * @note 这是一个STM32 HAL库的移植示例
 *       实际使用时需要根据具体的STM32型号和引脚配置进行修改
 */

#include "../hal/hal_uart.h"
#include <stdint.h>

/*
 * 如果使用STM32 HAL库，取消下面的注释
 * #include "stm32f4xx_hal.h"
 */

/* ============================================================================
 * 移植配置
 * ============================================================================ */

/* 定义使用的UART实例 */
#define PORT_UART_HANDLE        huart1

/* ============================================================================
 * 内部变量
 * ============================================================================ */

/* 接收回调函数指针 */
static void (*s_rx_callback)(uint8_t byte) = NULL;

/* 发送完成回调函数指针 */
static void (*s_tx_complete_callback)(void) = NULL;

/* 发送缓冲区（用于中断发送） */
static uint8_t s_tx_buffer[256];
static uint16_t s_tx_len = 0;
static volatile uint8_t s_tx_busy = 0;

/* ============================================================================
 * STM32 HAL库回调函数
 * ============================================================================ */

/*
 * 如果使用STM32 HAL库，取消下面的注释
 *
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t byte;

    if (huart == &PORT_UART_HANDLE) {
        // 读取接收到的字节
        byte = (uint8_t)(PORT_UART_HANDLE.Instance->DR & 0xFF);

        // 调用Modbus回调
        if (s_rx_callback != NULL) {
            s_rx_callback(byte);
        }

        // 重新启用接收中断
        HAL_UART_Receive_IT(&PORT_UART_HANDLE, &byte, 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &PORT_UART_HANDLE) {
        s_tx_busy = 0;

        // 调用发送完成回调
        if (s_tx_complete_callback != NULL) {
            s_tx_complete_callback();
        }
    }
}
*/

/* ============================================================================
 * 串口HAL接口实现
 * ============================================================================ */

/**
 * @brief 初始化串口
 */
static int port_uart_init(uint32_t baudrate, hal_uart_parity_t parity)
{
    /*
     * 如果使用STM32 HAL库，取消下面的注释并配置UART
     *
    UART_HandleTypeDef huart;
    uint32_t hal_parity;

    // 转换校验类型
    switch (parity) {
        case HAL_UART_PARITY_ODD:
            hal_parity = UART_PARITY_ODD;
            break;
        case HAL_UART_PARITY_EVEN:
            hal_parity = UART_PARITY_EVEN;
            break;
        default:
            hal_parity = UART_PARITY_NONE;
            break;
    }

    // 配置UART
    huart.Instance = USART1;
    huart.Init.BaudRate = baudrate;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = hal_parity;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart) != HAL_OK) {
        return -1;
    }

    // 启用接收中断
    uint8_t dummy;
    HAL_UART_Receive_IT(&PORT_UART_HANDLE, &dummy, 1);
    */

    /* 模拟初始化成功 */
    (void)baudrate;
    (void)parity;

    return 0;
}

/**
 * @brief 反初始化串口
 */
static void port_uart_deinit(void)
{
    /*
     * 如果使用STM32 HAL库，取消下面的注释
     *
    HAL_UART_DeInit(&PORT_UART_HANDLE);
    */
}

/**
 * @brief 发送数据（异步）
 */
static int port_uart_send(const uint8_t *data, uint16_t len)
{
    /*
     * 如果使用STM32 HAL库，取消下面的注释
     *
    if (s_tx_busy) {
        return -1;  // 忙
    }

    if (len > sizeof(s_tx_buffer)) {
        return -1;  // 缓冲区太小
    }

    // 复制数据到发送缓冲区
    memcpy(s_tx_buffer, data, len);
    s_tx_len = len;
    s_tx_busy = 1;

    // 启动发送
    if (HAL_UART_Transmit_IT(&PORT_UART_HANDLE, s_tx_buffer, len) != HAL_OK) {
        s_tx_busy = 0;
        return -1;
    }
    */

    /* 模拟发送成功 */
    (void)data;
    (void)len;

    /* 模拟发送完成回调 */
    if (s_tx_complete_callback != NULL) {
        s_tx_complete_callback();
    }

    return 0;
}

/**
 * @brief 设置接收回调函数
 */
static void port_uart_set_rx_callback(void (*callback)(uint8_t byte))
{
    s_rx_callback = callback;
}

/**
 * @brief 设置发送完成回调函数
 */
static void port_uart_set_tx_complete_callback(void (*callback)(void))
{
    s_tx_complete_callback = callback;
}

/* ============================================================================
 * HAL接口注册
 * ============================================================================ */

/* STM32串口HAL接口 */
const hal_uart_t hal_uart_stm32 = {
    .init = port_uart_init,
    .deinit = port_uart_deinit,
    .send = port_uart_send,
    .set_rx_callback = port_uart_set_rx_callback,
    .set_tx_complete_callback = port_uart_set_tx_complete_callback
};

/**
 * @brief 注册STM32串口HAL接口
 */
void port_uart_stm32_register(void)
{
    hal_uart_register(&hal_uart_stm32);
}
