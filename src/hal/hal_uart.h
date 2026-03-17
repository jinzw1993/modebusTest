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

/**
 * @brief Parity type definition
 */
typedef enum {
    HAL_UART_PARITY_NONE = 0,
    HAL_UART_PARITY_ODD  = 1,
    HAL_UART_PARITY_EVEN = 2
} hal_uart_parity_t;

/**
 * @brief UART HAL interface structure
 */
typedef struct hal_uart {
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
 * API Functions
 * ============================================================================ */

/**
 * @brief Register UART HAL interface
 */
int hal_uart_register(const hal_uart_t *hal);

/**
 * @brief Initialize UART
 */
int hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity);

/**
 * @brief Deinitialize UART
 */
void hal_uart_deinit(void);

/**
 * @brief Send data asynchronously
 */
int hal_uart_send(const uint8_t *data, uint16_t len);

/**
 * @brief Check if TX is busy
 */
int hal_uart_is_tx_busy(void);

/**
 * @brief Get RX error count
 */
uint32_t hal_uart_get_rx_errors(void);

/**
 * @brief Clear RX error count
 */
void hal_uart_clear_rx_errors(void);

/**
 * @brief Set RX byte callback
 */
void hal_uart_set_rx_callback(void (*callback)(uint8_t byte));

/**
 * @brief Set TX complete callback
 */
void hal_uart_set_tx_complete_callback(void (*callback)(void));

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */
