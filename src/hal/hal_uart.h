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
typedef struct {
    /* Initialization */
    int  (*init)(uint32_t baudrate, hal_uart_parity_t parity);
    void (*deinit)(void);

    /* Data transfer */
    int  (*send)(const uint8_t *data, uint16_t len);
    int  (*is_tx_busy)(void);

    /* Error handling */
    uint32_t (*get_rx_errors)(void);
    void     (*clear_rx_errors)(void);

    /* Callbacks */
    void (*set_rx_callback)(void (*callback)(uint8_t byte));
    void (*set_tx_complete_callback)(void (*callback)(void));
} hal_uart_t;

/* ============================================================================
 * API Functions
 * ============================================================================ */

/**
 * @brief Register UART HAL interface
 * @param hal Pointer to HAL interface structure
 * @return 0 on success, -1 on error
 */
int hal_uart_register(const hal_uart_t *hal);

/**
 * @brief Initialize UART
 * @param baudrate Baud rate (e.g., 9600, 19200, 115200)
 * @param parity Parity setting
 * @return 0 on success, -1 on error
 */
int hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity);

/**
 * @brief Deinitialize UART
 */
void hal_uart_deinit(void);

/**
 * @brief Send data asynchronously
 * @param data Data buffer
 * @param len Data length
 * @return 0 on success, -1 on error or busy
 */
int hal_uart_send(const uint8_t *data, uint16_t len);

/**
 * @brief Check if TX is busy
 * @return 1 if busy, 0 if idle
 */
int hal_uart_is_tx_busy(void);

/**
 * @brief Get RX error count
 * @return Error count
 */
uint32_t hal_uart_get_rx_errors(void);

/**
 * @brief Clear RX error count
 */
void hal_uart_clear_rx_errors(void);

/**
 * @brief Set RX byte callback
 * @param callback Callback function
 */
void hal_uart_set_rx_callback(void (*callback)(uint8_t byte));

/**
 * @brief Set TX complete callback
 * @param callback Callback function
 */
void hal_uart_set_tx_complete_callback(void (*callback)(void));

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */
