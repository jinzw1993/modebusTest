/**
 * @file hal_uart.c
 * @brief UART Hardware Abstraction Layer Implementation
 * @author Claude
 * @date 2026-03-17
 */

#include "hal_uart.h"
#include <stddef.h>

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

static const hal_uart_t *s_hal_uart = NULL;

/* ============================================================================
 * Register Function
 * ============================================================================ */

int hal_uart_register(const hal_uart_t *hal)
{
    if (hal == NULL) {
        return -1;
    }
    s_hal_uart = hal;
    return 0;
}

/* ============================================================================
 * API Implementation
 * ============================================================================ */

int hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity)
{
    if (s_hal_uart == NULL || s_hal_uart->init == NULL) {
        return -1;
    }
    return s_hal_uart->init(baudrate, parity);
}

void hal_uart_deinit(void)
{
    if (s_hal_uart != NULL && s_hal_uart->deinit != NULL) {
        s_hal_uart->deinit();
    }
}

int hal_uart_send(const uint8_t *data, uint16_t len)
{
    if (s_hal_uart == NULL || s_hal_uart->send == NULL) {
        return -1;
    }
    return s_hal_uart->send(data, len);
}

int hal_uart_is_tx_busy(void)
{
    if (s_hal_uart == NULL || s_hal_uart->is_tx_busy == NULL) {
        return 0;
    }
    return s_hal_uart->is_tx_busy();
}

uint32_t hal_uart_get_rx_errors(void)
{
    if (s_hal_uart == NULL || s_hal_uart->get_rx_errors == NULL) {
        return 0;
    }
    return s_hal_uart->get_rx_errors();
}

void hal_uart_clear_rx_errors(void)
{
    if (s_hal_uart != NULL && s_hal_uart->clear_rx_errors != NULL) {
        s_hal_uart->clear_rx_errors();
    }
}

void hal_uart_set_rx_callback(void (*callback)(uint8_t byte))
{
    if (s_hal_uart != NULL && s_hal_uart->set_rx_callback != NULL) {
        s_hal_uart->set_rx_callback(callback);
    }
}

void hal_uart_set_tx_complete_callback(void (*callback)(void))
{
    if (s_hal_uart != NULL && s_hal_uart->set_tx_complete_callback != NULL) {
        s_hal_uart->set_tx_complete_callback(callback);
    }
}
