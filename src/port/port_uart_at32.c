/**
 * @file port_uart_at32.c
 * @brief AT32F432串口移植实现
 * @author Claude
 * @date 2026-03-17
 *
 * @note 这是AT32F432标准外设库的移植实现
 *       使用USART1作为Modbus通信接口
 */

#include "../hal/hal_uart.h"
#include "at32f423.h"
#include "at32f423_usart.h"
#include "at32f423_gpio.h"
#include "at32f423_crm.h"
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * 移植配置
 * ============================================================================ */

/* 使用USART1作为Modbus通信口 */
#define PORT_USART              USART1
#define PORT_USART_IRQn         USART1_IRQn

/* 引脚配置 (PA9=TX, PA10=RX) */
#define PORT_USART_TX_GPIO      GPIOA
#define PORT_USART_TX_PIN       GPIO_PINS_9
#define PORT_USART_RX_GPIO      GPIOA
#define PORT_USART_RX_PIN       GPIO_PINS_10

/* 时钟配置 */
#define PORT_USART_CLK          CRM_USART1_PERIPH_CLOCK
#define PORT_USART_GPIO_CLK     CRM_GPIOA_PERIPH_CLOCK

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
static uint16_t s_tx_index = 0;
static volatile uint8_t s_tx_busy = 0;

/* 接收字节缓冲 */
static uint8_t s_rx_byte;

/* ============================================================================
 * AT32中断服务函数
 * ============================================================================ */

/**
 * @brief USART1中断服务函数
 */
void USART1_IRQHandler(void)
{
    /* 接收中断 */
    if (usart_flag_get(PORT_USART, USART_RDBF_FLAG) != RESET) {
        /* 读取接收到的字节 */
        s_rx_byte = usart_data_receive(PORT_USART);

        /* 调用Modbus回调 */
        if (s_rx_callback != NULL) {
            s_rx_callback(s_rx_byte);
        }
    }

    /* 发送完成中断 */
    if (usart_flag_get(PORT_USART, USART_TDC_FLAG) != RESET) {
        usart_interrupt_enable(PORT_USART, USART_TDC_INT, FALSE);

        if (s_tx_index < s_tx_len) {
            /* 继续发送下一个字节 */
            usart_data_transmit(PORT_USART, s_tx_buffer[s_tx_index++]);
            usart_interrupt_enable(PORT_USART, USART_TDC_INT, TRUE);
        } else {
            /* 发送完成 */
            s_tx_busy = 0;
            s_tx_index = 0;
            s_tx_len = 0;

            /* 调用发送完成回调 */
            if (s_tx_complete_callback != NULL) {
                s_tx_complete_callback();
            }
        }
    }
}

/* ============================================================================
 * 串口HAL接口实现
 * ============================================================================ */

/**
 * @brief 初始化串口
 */
static int port_uart_init(uint32_t baudrate, hal_uart_parity_t parity)
{
    gpio_init_type gpio_init_struct;
    usart_init_type usart_init_struct;

    /* 使能时钟 */
    crm_periph_clock_enable(PORT_USART_CLK, TRUE);
    crm_periph_clock_enable(PORT_USART_GPIO_CLK, TRUE);

    /* 配置TX引脚为复用推挽输出 */
    gpio_init_struct.gpio_pins = PORT_USART_TX_PIN;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(PORT_USART_TX_GPIO, &gpio_init_struct);

    /* 配置RX引脚为浮空输入 */
    gpio_init_struct.gpio_pins = PORT_USART_RX_PIN;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(PORT_USART_RX_GPIO, &gpio_init_struct);

    /* 复位USART */
    usart_reset(PORT_USART);

    /* 配置USART参数 */
    usart_init_struct.baudrate = baudrate;
    usart_init_struct.word_length = USART_WL_8B;
    usart_init_struct.stop_bits = USART_STB_1BIT;

    /* 校验位配置 */
    switch (parity) {
        case HAL_UART_PARITY_ODD:
            usart_init_struct.parity = USART_PA_ODD;
            break;
        case HAL_UART_PARITY_EVEN:
            usart_init_struct.parity = USART_PA_EVEN;
            break;
        default:
            usart_init_struct.parity = USART_PA_NONE;
            break;
    }

    usart_init_struct.mode = USART_MODE_TX_RX;
    usart_init_struct.hardware_flow_control = USART_FLOW_CONTROL_NONE;

    usart_init(PORT_USART, &usart_init_struct);

    /* 配置中断优先级 */
    nvic_irq_enable(PORT_USART_IRQn, 0, 0);

    /* 使能接收中断 */
    usart_interrupt_enable(PORT_USART, USART_RD_INT, TRUE);

    /* 使能USART */
    usart_enable(PORT_USART, TRUE);

    return 0;
}

/**
 * @brief 反初始化串口
 */
static void port_uart_deinit(void)
{
    usart_reset(PORT_USART);
    usart_enable(PORT_USART, FALSE);
    nvic_irq_enable(PORT_USART_IRQn, FALSE);
}

/**
 * @brief 发送数据（异步）
 */
static int port_uart_send(const uint8_t *data, uint16_t len)
{
    if (s_tx_busy) {
        return -1;  /* 忙 */
    }

    if (len > sizeof(s_tx_buffer)) {
        return -1;  /* 缓冲区太小 */
    }

    if (len == 0) {
        return 0;
    }

    /* 复制数据到发送缓冲区 */
    memcpy(s_tx_buffer, data, len);
    s_tx_len = len;
    s_tx_index = 0;
    s_tx_busy = 1;

    /* 启动发送（发送第一个字节） */
    usart_data_transmit(PORT_USART, s_tx_buffer[s_tx_index++]);
    usart_interrupt_enable(PORT_USART, USART_TDC_INT, TRUE);

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

/* AT32串口HAL接口 */
const hal_uart_t hal_uart_at32 = {
    .init = port_uart_init,
    .deinit = port_uart_deinit,
    .send = port_uart_send,
    .set_rx_callback = port_uart_set_rx_callback,
    .set_tx_complete_callback = port_uart_set_tx_complete_callback
};

/**
 * @brief 注册AT32串口HAL接口
 */
void port_uart_at32_register(void)
{
    hal_uart_register(&hal_uart_at32);
}
