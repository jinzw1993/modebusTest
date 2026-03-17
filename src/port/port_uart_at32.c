/**
 * @file port_uart_at32.c
 * @brief AT32F423 USART driver for Modbus RTU
 * @author Claude
 * @date 2026-03-17
 */

#include "../hal/hal_uart.h"
#include "modbus_config.h"
#include "at32f423.h"
#include "at32f423_usart.h"
#include "at32f423_gpio.h"
#include "at32f423_crm.h"
#include "at32f423_misc.h"
#include <string.h>

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define PORT_USARTx             USART1
#define PORT_USARTx_IRQn        USART1_IRQn
#define PORT_USARTx_CLK         CRM_USART1_PERIPH_CLOCK

#define PORT_USARTx_TX_GPIO     GPIOA
#define PORT_USARTx_TX_PIN      GPIO_PINS_9
#define PORT_USARTx_TX_PIN_SRC  GPIO_PINS_SOURCE9

#define PORT_USARTx_RX_GPIO     GPIOA
#define PORT_USARTx_RX_PIN      GPIO_PINS_10
#define PORT_USARTx_RX_PIN_SRC  GPIO_PINS_SOURCE10

#define PORT_USARTx_GPIO_CLK    CRM_GPIOA_PERIPH_CLOCK

/* RS485 control (set to 1 to enable) */
#define PORT_RS485_ENABLE       0

#if PORT_RS485_ENABLE
#define PORT_RS485_DE_GPIO      GPIOA
#define PORT_RS485_DE_PIN       GPIO_PINS_8
#define RS485_TX_MODE()         gpio_bits_set(PORT_RS485_DE_GPIO, PORT_RS485_DE_PIN)
#define RS485_RX_MODE()         gpio_bits_reset(PORT_RS485_DE_GPIO, PORT_RS485_DE_PIN)
#else
#define RS485_TX_MODE()
#define RS485_RX_MODE()
#endif

#ifndef MB_BUFFER_SIZE
#define MB_BUFFER_SIZE          256
#endif

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

static void (*s_rx_callback)(uint8_t byte) = NULL;
static void (*s_tx_complete_callback)(void) = NULL;

static uint8_t  s_tx_buffer[MB_BUFFER_SIZE];
static uint16_t s_tx_len = 0;
static uint16_t s_tx_pos = 0;
static volatile uint8_t s_tx_busy = 0;
static volatile uint32_t s_rx_errors = 0;

/* ============================================================================
 * RS485 Initialization
 * ============================================================================ */

#if PORT_RS485_ENABLE
static void rs485_init(void)
{
    gpio_init_type gpio;
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    gpio.gpio_pins   = PORT_RS485_DE_PIN;
    gpio.gpio_mode   = GPIO_MODE_OUTPUT;
    gpio.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio.gpio_pull   = GPIO_PULL_NONE;
    gpio.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(PORT_RS485_DE_GPIO, &gpio);
    RS485_RX_MODE();
}
#endif

/* ============================================================================
 * Interrupt Handler
 * ============================================================================ */

void USART1_IRQHandler(void)
{
    uint8_t rx_byte;

    /* RX interrupt */
    if (usart_flag_get(PORT_USARTx, USART_RDBF_FLAG) != RESET) {
        rx_byte = (uint8_t)usart_data_receive(PORT_USARTx);
        if (s_rx_callback) {
            s_rx_callback(rx_byte);
        }
    }

    /* Error interrupts */
    if (usart_flag_get(PORT_USARTx, USART_ROERR_FLAG) != RESET) {
        usart_flag_clear(PORT_USARTx, USART_ROERR_FLAG);
        s_rx_errors++;
    }
    if (usart_flag_get(PORT_USARTx, USART_PERR_FLAG) != RESET) {
        usart_flag_clear(PORT_USARTx, USART_PERR_FLAG);
        s_rx_errors++;
    }
    if (usart_flag_get(PORT_USARTx, USART_FERR_FLAG) != RESET) {
        usart_flag_clear(PORT_USARTx, USART_FERR_FLAG);
        s_rx_errors++;
    }
    if (usart_flag_get(PORT_USARTx, USART_NERR_FLAG) != RESET) {
        usart_flag_clear(PORT_USARTx, USART_NERR_FLAG);
        s_rx_errors++;
    }

    /* TX complete interrupt */
    if (usart_flag_get(PORT_USARTx, USART_TDC_FLAG) != RESET) {
        usart_interrupt_enable(PORT_USARTx, USART_TDC_INT, FALSE);

        if (s_tx_pos < s_tx_len) {
            usart_data_transmit(PORT_USARTx, s_tx_buffer[s_tx_pos++]);
            usart_interrupt_enable(PORT_USARTx, USART_TDC_INT, TRUE);
        } else {
            s_tx_busy = 0;
            s_tx_pos = 0;
            s_tx_len = 0;

            RS485_RX_MODE();

            if (s_tx_complete_callback) {
                s_tx_complete_callback();
            }
        }
    }
}

/* ============================================================================
 * HAL Implementation
 * ============================================================================ */

static int port_uart_init(uint32_t baudrate, hal_uart_parity_t parity)
{
    gpio_init_type gpio;
    usart_data_bit_num_type data_bits = USART_DATA_8BITS;
    usart_stop_bit_num_type stop_bits = USART_STOP_1_BIT;
    usart_parity_selection_type parity_sel = USART_PARITY_NONE;

    /* Enable clocks */
    crm_periph_clock_enable(PORT_USARTx_GPIO_CLK, TRUE);
    crm_periph_clock_enable(PORT_USARTx_CLK, TRUE);

    /* Configure TX pin */
    gpio_pin_mux_config(PORT_USARTx_TX_GPIO, PORT_USARTx_TX_PIN_SRC, GPIO_MUX_7);
    gpio.gpio_pins   = PORT_USARTx_TX_PIN;
    gpio.gpio_mode   = GPIO_MODE_MUX;
    gpio.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio.gpio_pull   = GPIO_PULL_UP;
    gpio.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(PORT_USARTx_TX_GPIO, &gpio);

    /* Configure RX pin */
    gpio_pin_mux_config(PORT_USARTx_RX_GPIO, PORT_USARTx_RX_PIN_SRC, GPIO_MUX_7);
    gpio.gpio_pins   = PORT_USARTx_RX_PIN;
    gpio.gpio_mode   = GPIO_MODE_MUX;
    gpio.gpio_pull   = GPIO_PULL_UP;
    gpio_init(PORT_USARTx_RX_GPIO, &gpio);

#if PORT_RS485_ENABLE
    rs485_init();
#endif

    /* Reset USART */
    usart_reset(PORT_USARTx);

    /* Configure parity and stop bits */
    switch (parity) {
        case HAL_UART_PARITY_ODD:
            parity_sel = USART_PARITY_ODD;
            stop_bits = USART_STOP_1_BIT;
            break;
        case HAL_UART_PARITY_EVEN:
            parity_sel = USART_PARITY_EVEN;
            stop_bits = USART_STOP_1_BIT;
            break;
        default:
            parity_sel = USART_PARITY_NONE;
            stop_bits = USART_STOP_2_BIT;  /* Modbus: no parity = 2 stop bits */
            break;
    }

    /* Initialize USART */
    usart_init(PORT_USARTx, baudrate, data_bits, stop_bits);
    usart_parity_selection_config(PORT_USARTx, parity_sel);
    usart_transmitter_enable(PORT_USARTx, TRUE);
    usart_receiver_enable(PORT_USARTx, TRUE);

    /* Configure NVIC */
    nvic_irq_enable(PORT_USARTx_IRQn, 0, 0);

    /* Enable RX interrupt */
    usart_interrupt_enable(PORT_USARTx, USART_RDBF_INT, TRUE);

    /* Enable USART */
    usart_enable(PORT_USARTx, TRUE);

    return 0;
}

static void port_uart_deinit(void)
{
    usart_interrupt_enable(PORT_USARTx, USART_RDBF_INT, FALSE);
    usart_interrupt_enable(PORT_USARTx, USART_TDC_INT, FALSE);
    usart_enable(PORT_USARTx, FALSE);
    usart_reset(PORT_USARTx);
    nvic_irq_disable(PORT_USARTx_IRQn);
}

static int port_uart_send(const uint8_t *data, uint16_t len)
{
    if (s_tx_busy || data == NULL || len == 0 || len > MB_BUFFER_SIZE) {
        return -1;
    }

    memcpy(s_tx_buffer, data, len);
    s_tx_len = len;
    s_tx_pos = 0;
    s_tx_busy = 1;

    RS485_TX_MODE();

    usart_data_transmit(PORT_USARTx, s_tx_buffer[s_tx_pos++]);
    usart_interrupt_enable(PORT_USARTx, USART_TDC_INT, TRUE);

    return 0;
}

static int port_uart_is_tx_busy(void)
{
    return s_tx_busy;
}

static uint32_t port_uart_get_rx_errors(void)
{
    return s_rx_errors;
}

static void port_uart_clear_rx_errors(void)
{
    s_rx_errors = 0;
}

static void port_uart_set_rx_callback(void (*callback)(uint8_t byte))
{
    s_rx_callback = callback;
}

static void port_uart_set_tx_complete_callback(void (*callback)(void))
{
    s_tx_complete_callback = callback;
}

/* ============================================================================
 * HAL Registration
 * ============================================================================ */

const hal_uart_t hal_uart_at32 = {
    .init = port_uart_init,
    .deinit = port_uart_deinit,
    .send = port_uart_send,
    .is_tx_busy = port_uart_is_tx_busy,
    .get_rx_errors = port_uart_get_rx_errors,
    .clear_rx_errors = port_uart_clear_rx_errors,
    .set_rx_callback = port_uart_set_rx_callback,
    .set_tx_complete_callback = port_uart_set_tx_complete_callback
};

void port_uart_at32_register(void)
{
    hal_uart_register(&hal_uart_at32);
}
