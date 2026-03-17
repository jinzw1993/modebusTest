/**
 * @file port_uart_at32.c
 * @brief AT32F423 UART port implementation
 * @author Claude
 * @date 2026-03-17
 */

#include "../hal/hal_uart.h"
#include "at32f423.h"
#include "at32f423_usart.h"
#include "at32f423_gpio.h"
#include "at32f423_crm.h"
#include <stddef.h>
#include <string.h>

/* ============================================================================
 * Port Configuration
 * ============================================================================ */

/* Use USART1 as Modbus communication port */
#define PORT_USART              USART1
#define PORT_USART_IRQn         USART1_IRQn

/* Pin configuration (PA9=TX, PA10=RX) */
#define PORT_USART_TX_GPIO      GPIOA
#define PORT_USART_TX_PIN       GPIO_PINS_9
#define PORT_USART_RX_GPIO      GPIOA
#define PORT_USART_RX_PIN       GPIO_PINS_10

/* Clock configuration */
#define PORT_USART_CLK          CRM_USART1_PERIPH_CLOCK
#define PORT_USART_GPIO_CLK     CRM_GPIOA_PERIPH_CLOCK

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

/* RX callback function pointer */
static void (*s_rx_callback)(uint8_t byte) = NULL;

/* TX complete callback function pointer */
static void (*s_tx_complete_callback)(void) = NULL;

/* TX buffer (for interrupt transmission) */
static uint8_t s_tx_buffer[256];
static uint16_t s_tx_len = 0;
static uint16_t s_tx_index = 0;
static volatile uint8_t s_tx_busy = 0;

/* RX byte buffer */
static uint8_t s_rx_byte;

/* ============================================================================
 * AT32 Interrupt Service Function
 * ============================================================================ */

/**
 * @brief USART1 interrupt handler
 */
void USART1_IRQHandler(void)
{
    /* RX interrupt */
    if (usart_flag_get(PORT_USART, USART_RDBF_FLAG) != RESET) {
        /* Read received byte */
        s_rx_byte = usart_data_receive(PORT_USART);

        /* Call Modbus callback */
        if (s_rx_callback != NULL) {
            s_rx_callback(s_rx_byte);
        }
    }

    /* TX complete interrupt */
    if (usart_flag_get(PORT_USART, USART_TDC_FLAG) != RESET) {
        usart_interrupt_enable(PORT_USART, USART_TDC_INT, FALSE);

        if (s_tx_index < s_tx_len) {
            /* Continue sending next byte */
            usart_data_transmit(PORT_USART, s_tx_buffer[s_tx_index++]);
            usart_interrupt_enable(PORT_USART, USART_TDC_INT, TRUE);
        } else {
            /* Transmission complete */
            s_tx_busy = 0;
            s_tx_index = 0;
            s_tx_len = 0;

            /* Call TX complete callback */
            if (s_tx_complete_callback != NULL) {
                s_tx_complete_callback();
            }
        }
    }
}

/* ============================================================================
 * UART HAL Interface Implementation
 * ============================================================================ */

/**
 * @brief Initialize UART
 */
static int port_uart_init(uint32_t baudrate, hal_uart_parity_t parity)
{
    gpio_init_type gpio_init_struct;

    /* Enable clocks */
    crm_periph_clock_enable(PORT_USART_CLK, TRUE);
    crm_periph_clock_enable(PORT_USART_GPIO_CLK, TRUE);

    /* Configure TX pin as alternate function push-pull */
    gpio_init_struct.gpio_pins = PORT_USART_TX_PIN;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(PORT_USART_TX_GPIO, &gpio_init_struct);

    /* Configure RX pin as floating input */
    gpio_init_struct.gpio_pins = PORT_USART_RX_PIN;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(PORT_USART_RX_GPIO, &gpio_init_struct);

    /* Reset USART */
    usart_reset(PORT_USART);

    /* Configure USART parameters */
    usart_init(PORT_USART, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);

    /* Configure parity */
    switch (parity) {
        case HAL_UART_PARITY_ODD:
            usart_parity_selection_config(PORT_USART, USART_PARITY_ODD);
            break;
        case HAL_UART_PARITY_EVEN:
            usart_parity_selection_config(PORT_USART, USART_PARITY_EVEN);
            break;
        default:
            usart_parity_selection_config(PORT_USART, USART_PARITY_NONE);
            break;
    }

    /* Enable TX and RX */
    usart_transmitter_enable(PORT_USART, TRUE);
    usart_receiver_enable(PORT_USART, TRUE);

    /* Configure interrupt priority */
    nvic_irq_enable(PORT_USART_IRQn, 0, 0);

    /* Enable RX interrupt */
    usart_interrupt_enable(PORT_USART, USART_RDBF_INT, TRUE);

    /* Enable USART */
    usart_enable(PORT_USART, TRUE);

    return 0;
}

/**
 * @brief Deinitialize UART
 */
static void port_uart_deinit(void)
{
    usart_reset(PORT_USART);
    usart_enable(PORT_USART, FALSE);
    nvic_irq_disable(PORT_USART_IRQn);
}

/**
 * @brief Send data (async)
 */
static int port_uart_send(const uint8_t *data, uint16_t len)
{
    if (s_tx_busy) {
        return -1;  /* Busy */
    }

    if (len > sizeof(s_tx_buffer)) {
        return -1;  /* Buffer too small */
    }

    if (len == 0) {
        return 0;
    }

    /* Copy data to TX buffer */
    memcpy(s_tx_buffer, data, len);
    s_tx_len = len;
    s_tx_index = 0;
    s_tx_busy = 1;

    /* Start transmission (send first byte) */
    usart_data_transmit(PORT_USART, s_tx_buffer[s_tx_index++]);
    usart_interrupt_enable(PORT_USART, USART_TDC_INT, TRUE);

    return 0;
}

/**
 * @brief Set RX callback
 */
static void port_uart_set_rx_callback(void (*callback)(uint8_t byte))
{
    s_rx_callback = callback;
}

/**
 * @brief Set TX complete callback
 */
static void port_uart_set_tx_complete_callback(void (*callback)(void))
{
    s_tx_complete_callback = callback;
}

/* ============================================================================
 * HAL Interface Registration
 * ============================================================================ */

/* AT32 UART HAL interface */
const hal_uart_t hal_uart_at32 = {
    .init = port_uart_init,
    .deinit = port_uart_deinit,
    .send = port_uart_send,
    .set_rx_callback = port_uart_set_rx_callback,
    .set_tx_complete_callback = port_uart_set_tx_complete_callback
};

/**
 * @brief Register AT32 UART HAL interface
 */
void port_uart_at32_register(void)
{
    hal_uart_register(&hal_uart_at32);
}
