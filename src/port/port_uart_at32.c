/**
 * @file port_uart_at32.c
 * @brief AT32F423 USART driver for Modbus RTU
 * @author Claude
 * @date 2026-03-17
 *
 * @note Features:
 *       - Interrupt-driven TX/RX
 *       - RS485 direction control (optional)
 *       - Error handling (overrun, parity, framing)
 *       - Configurable GPIO pins
 */

#include "../hal/hal_uart.h"
#include "at32f423.h"
#include "at32f423_usart.h"
#include "at32f423_gpio.h"
#include "at32f423_crm.h"
#include "at32f423_misc.h"
#include <stddef.h>
#include <string.h>

/* ============================================================================
 * Configuration
 * ============================================================================ */

/* USART selection */
#define PORT_USARTx             USART1
#define PORT_USARTx_IRQn        USART1_IRQn
#define PORT_USARTx_CLK         CRM_USART1_PERIPH_CLOCK
#define PORT_USARTx_APBx_CLK    CRM_USART1_PERIPH_CLOCK

/* GPIO configuration */
#define PORT_USARTx_TX_GPIO     GPIOA
#define PORT_USARTx_TX_PIN      GPIO_PINS_9
#define PORT_USARTx_TX_PIN_SRC  GPIO_PINS_SOURCE9

#define PORT_USARTx_RX_GPIO     GPIOA
#define PORT_USARTx_RX_PIN      GPIO_PINS_10
#define PORT_USARTx_RX_PIN_SRC  GPIO_PINS_SOURCE10

#define PORT_USARTx_GPIO_CLK    CRM_GPIOA_PERIPH_CLOCK

/* RS485 DE/RE control (optional, set to 0 to disable) */
#define PORT_RS485_ENABLE       0

#if PORT_RS485_ENABLE
#define PORT_RS485_DE_GPIO      GPIOA
#define PORT_RS485_DE_PIN       GPIO_PINS_8
#define PORT_RS485_DE_GPIO_CLK  CRM_GPIOA_PERIPH_CLOCK

#define RS485_TX_MODE()         gpio_bits_set(PORT_RS485_DE_GPIO, PORT_RS485_DE_PIN)
#define RS485_RX_MODE()         gpio_bits_reset(PORT_RS485_DE_GPIO, PORT_RS485_DE_PIN)
#else
#define RS485_TX_MODE()
#define RS485_RX_MODE()
#endif

/* Interrupt priority */
#define PORT_USARTx_PREEMPT_PRI  0
#define PORT_USARTx_SUB_PRI      0

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

/* Callbacks */
static void (*s_rx_callback)(uint8_t byte) = NULL;
static void (*s_tx_complete_callback)(void) = NULL;

/* TX state machine */
static uint8_t  s_tx_buffer[MB_RTU_BUFFER_SIZE];
static uint16_t s_tx_len = 0;
static uint16_t s_tx_pos = 0;
static volatile uint8_t s_tx_busy = 0;

/* Error tracking */
static volatile uint32_t s_rx_errors = 0;

/* ============================================================================
 * RS485 Control
 * ============================================================================ */

#if PORT_RS485_ENABLE
/**
 * @brief Initialize RS485 direction control pin
 */
static void rs485_init(void)
{
    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(PORT_RS485_DE_GPIO_CLK, TRUE);

    gpio_init_struct.gpio_pins   = PORT_RS485_DE_PIN;
    gpio_init_struct.gpio_mode   = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull   = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(PORT_RS485_DE_GPIO, &gpio_init_struct);

    /* Default to RX mode */
    RS485_RX_MODE();
}
#endif

/* ============================================================================
 * Interrupt Handler
 * ============================================================================ */

/**
 * @brief USART1 global interrupt handler
 */
void USART1_IRQHandler(void)
{
    uint32_t int_flag;
    uint8_t  rx_data;

    /* ========== RX Processing ========== */

    /* Receive data buffer full interrupt */
    int_flag = usart_flag_get(PORT_USARTx, USART_RDBF_FLAG);
    if (int_flag != RESET) {
        rx_data = (uint8_t)usart_data_receive(PORT_USARTx);

        if (s_rx_callback != NULL) {
            s_rx_callback(rx_data);
        }
    }

    /* Overrun error */
    int_flag = usart_flag_get(PORT_USARTx, USART_ROERR_FLAG);
    if (int_flag != RESET) {
        usart_flag_clear(PORT_USARTx, USART_ROERR_FLAG);
        s_rx_errors++;
    }

    /* Parity error */
    int_flag = usart_flag_get(PORT_USARTx, USART_PERR_FLAG);
    if (int_flag != RESET) {
        usart_flag_clear(PORT_USARTx, USART_PERR_FLAG);
        s_rx_errors++;
    }

    /* Framing error */
    int_flag = usart_flag_get(PORT_USARTx, USART_FERR_FLAG);
    if (int_flag != RESET) {
        usart_flag_clear(PORT_USARTx, USART_FERR_FLAG);
        s_rx_errors++;
    }

    /* Noise error */
    int_flag = usart_flag_get(PORT_USARTx, USART_NERR_FLAG);
    if (int_flag != RESET) {
        usart_flag_clear(PORT_USARTx, USART_NERR_FLAG);
        s_rx_errors++;
    }

    /* ========== TX Processing ========== */

    /* Transmission complete interrupt */
    int_flag = usart_flag_get(PORT_USARTx, USART_TDC_FLAG);
    if (int_flag != RESET) {
        if (s_tx_pos < s_tx_len) {
            /* Send next byte */
            usart_data_transmit(PORT_USARTx, s_tx_buffer[s_tx_pos++]);
        } else {
            /* All bytes sent */
            usart_interrupt_enable(PORT_USARTx, USART_TDC_INT, FALSE);

            s_tx_busy = 0;
            s_tx_pos = 0;
            s_tx_len = 0;

#if PORT_RS485_ENABLE
            /* Switch back to RX mode after TX complete */
            RS485_RX_MODE();
#endif
            if (s_tx_complete_callback != NULL) {
                s_tx_complete_callback();
            }
        }
    }
}

/* ============================================================================
 * HAL Interface Implementation
 * ============================================================================ */

/**
 * @brief Initialize USART
 * @param baudrate Baud rate (e.g., 9600, 19200, 115200)
 * @param parity Parity setting
 * @return 0 on success, -1 on error
 */
static int port_uart_init(uint32_t baudrate, hal_uart_parity_t parity)
{
    gpio_init_type gpio_init_struct;
    usart_data_bit_num_type data_bits;
    usart_parity_selection_type parity_sel;
    usart_stop_bit_num_type stop_bits;

    /* Enable peripheral clocks */
    crm_periph_clock_enable(PORT_USARTx_GPIO_CLK, TRUE);
    crm_periph_clock_enable(PORT_USARTx_CLK, TRUE);

    /* Configure TX pin as alternate function */
    gpio_pin_mux_config(PORT_USARTx_TX_GPIO, PORT_USARTx_TX_PIN_SRC, GPIO_MUX_7);
    gpio_init_struct.gpio_pins   = PORT_USARTx_TX_PIN;
    gpio_init_struct.gpio_mode   = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull   = GPIO_PULL_UP;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(PORT_USARTx_TX_GPIO, &gpio_init_struct);

    /* Configure RX pin as alternate function */
    gpio_pin_mux_config(PORT_USARTx_RX_GPIO, PORT_USARTx_RX_PIN_SRC, GPIO_MUX_7);
    gpio_init_struct.gpio_pins   = PORT_USARTx_RX_PIN;
    gpio_init_struct.gpio_mode   = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pull   = GPIO_PULL_UP;
    gpio_init(PORT_USARTx_RX_GPIO, &gpio_init_struct);

#if PORT_RS485_ENABLE
    rs485_init();
#endif

    /* Reset USART */
    usart_reset(PORT_USARTx);

    /* Configure data bits based on parity
     * Modbus RTU: 8 data bits + parity = 8-bit word with parity
     *             8 data bits + no parity = 8-bit word (force 2 stop bits)
     */
    if (parity == HAL_UART_PARITY_NONE) {
        data_bits = USART_DATA_8BITS;
        stop_bits = USART_STOP_2_BIT;  /* Modbus spec: no parity requires 2 stop bits */
    } else {
        data_bits = USART_DATA_9BITS;  /* 8 data + 1 parity = 9-bit word */
        stop_bits = USART_STOP_1_BIT;
    }

    /* Configure parity */
    switch (parity) {
        case HAL_UART_PARITY_ODD:
            parity_sel = USART_PARITY_ODD;
            break;
        case HAL_UART_PARITY_EVEN:
            parity_sel = USART_PARITY_EVEN;
            break;
        default:
            parity_sel = USART_PARITY_NONE;
            break;
    }

    /* Initialize USART */
    usart_baudrate_set(PORT_USARTx, baudrate);
    usart_word_length_set(PORT_USARTx, data_bits);
    usart_stop_bit_set(PORT_USARTx, stop_bits);
    usart_parity_set(PORT_USARTx, parity_sel);

    /* Enable TX/RX */
    usart_transmitter_enable(PORT_USARTx, TRUE);
    usart_receiver_enable(PORT_USARTx, TRUE);

    /* Configure NVIC */
    nvic_irq_enable(PORT_USARTx_IRQn, PORT_USARTx_PREEMPT_PRI, PORT_USARTx_SUB_PRI);

    /* Enable RX interrupt */
    usart_interrupt_enable(PORT_USARTx, USART_RDBF_INT, TRUE);
    usart_interrupt_enable(PORT_USARTx, USART_ERR_INT, TRUE);

    /* Enable USART */
    usart_enable(PORT_USARTx, TRUE);

    /* Clear initial flags */
    usart_flag_clear(PORT_USARTx, USART_ROERR_FLAG);
    usart_flag_clear(PORT_USARTx, USART_PERR_FLAG);
    usart_flag_clear(PORT_USARTx, USART_FERR_FLAG);
    usart_flag_clear(PORT_USARTx, USART_NERR_FLAG);

    return 0;
}

/**
 * @brief Deinitialize USART
 */
static void port_uart_deinit(void)
{
    usart_interrupt_enable(PORT_USARTx, USART_RDBF_INT, FALSE);
    usart_interrupt_enable(PORT_USARTx, USART_TDC_INT, FALSE);
    usart_interrupt_enable(PORT_USARTx, USART_ERR_INT, FALSE);
    usart_enable(PORT_USARTx, FALSE);
    usart_reset(PORT_USARTx);
    nvic_irq_disable(PORT_USARTx_IRQn);
}

/**
 * @brief Send data asynchronously
 * @param data Data buffer
 * @param len Data length
 * @return 0 on success, -1 on error or busy
 */
static int port_uart_send(const uint8_t *data, uint16_t len)
{
    /* Check busy */
    if (s_tx_busy) {
        return -1;
    }

    /* Validate parameters */
    if (data == NULL || len == 0) {
        return -1;
    }

    if (len > sizeof(s_tx_buffer)) {
        return -1;
    }

    /* Copy data to TX buffer */
    memcpy(s_tx_buffer, data, len);
    s_tx_len = len;
    s_tx_pos = 0;
    s_tx_busy = 1;

#if PORT_RS485_ENABLE
    /* Switch to TX mode */
    RS485_TX_MODE();
#endif

    /* Start transmission - send first byte */
    usart_data_transmit(PORT_USARTx, s_tx_buffer[s_tx_pos++]);

    /* Enable TX complete interrupt for remaining bytes */
    usart_interrupt_enable(PORT_USARTx, USART_TDC_INT, TRUE);

    return 0;
}

/**
 * @brief Check if TX is busy
 * @return 1 if busy, 0 if idle
 */
static int port_uart_is_tx_busy(void)
{
    return s_tx_busy;
}

/**
 * @brief Get RX error count
 * @return Error count
 */
static uint32_t port_uart_get_rx_errors(void)
{
    return s_rx_errors;
}

/**
 * @brief Clear RX error count
 */
static void port_uart_clear_rx_errors(void)
{
    s_rx_errors = 0;
}

/**
 * @brief Set RX byte callback
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
