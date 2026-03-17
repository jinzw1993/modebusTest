/**
 * @file hal_uart.h
 * @brief UART Hardware Abstraction Layer Interface
 * @author Claude
 * @date 2026-03-17
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Types
 * ============================================================================ */

typedef enum {
    HAL_UART_PARITY_NONE = 0,
    HAL_UART_PARITY_ODD  = 1,
    HAL_UART_PARITY_EVEN = 2
} hal_uart_parity_t;

typedef struct {
    int  (*init)(uint32_t baudrate, hal_uart_parity_t parity);
    void (*deinit)(void);
    int  (*send)(const uint8_t *data, uint16_t len);
    int  (*is_tx_busy)(void);
    uint32_t (*get_rx_errors)(void);
    void (*clear_rx_errors)(void);
    void (*set_rx_callback)(void (*callback)(uint8_t byte));
    void (*set_tx_complete_callback)(void (*callback)(void));
} hal_uart_t;

/* ============================================================================
 * Registration API
 * ============================================================================ */

/**
 * @brief Register UART HAL driver
 * @param hal Pointer to HAL interface
 * @return 0 on success, -1 on error
 */
int hal_uart_register(const hal_uart_t *hal);

/* ============================================================================
 * HAL API Functions
 * ============================================================================ */

int  hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity);
void hal_uart_deinit(void);
int  hal_uart_send(const uint8_t *data, uint16_t len);
int  hal_uart_is_tx_busy(void);
uint32_t hal_uart_get_rx_errors(void);
void hal_uart_clear_rx_errors(void);
void hal_uart_set_rx_callback(void (*callback)(uint8_t byte));
void hal_uart_set_tx_complete_callback(void (*callback)(void));

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */
