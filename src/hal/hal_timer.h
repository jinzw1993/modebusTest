/**
 * @file hal_timer.h
 * @brief Timer Hardware Abstraction Layer Interface
 * @author Claude
 * @date 2026-03-17
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Types
 * ============================================================================ */

/**
 * @brief Timer HAL interface structure
 */
typedef struct {
    /* Initialization */
    int  (*init)(uint32_t timeout_ms);
    void (*deinit)(void);

    /* Control */
    void (*start)(void);
    void (*stop)(void);
    void (*reset)(void);

    /* Configuration */
    void (*set_timeout)(uint32_t timeout_ms);
    void (*set_callback)(void (*callback)(void));

    /* Status (polling mode) */
    bool (*is_expired)(void);
    bool (*is_running)(void);

    /* Counter access */
    uint32_t (*get_counter)(void);
    uint32_t (*get_elapsed_ms)(void);
} hal_timer_t;

/* ============================================================================
 * API Functions
 * ============================================================================ */

/**
 * @brief Register timer HAL interface
 * @param hal Pointer to HAL interface structure
 * @return 0 on success, -1 on error
 */
int hal_timer_register(const hal_timer_t *hal);

/**
 * @brief Initialize timer
 * @param timeout_ms Timeout value in milliseconds
 * @return 0 on success, -1 on error
 */
int hal_timer_init(uint32_t timeout_ms);

/**
 * @brief Deinitialize timer
 */
void hal_timer_deinit(void);

/**
 * @brief Start timer (one-shot mode)
 */
void hal_timer_start(void);

/**
 * @brief Stop timer
 */
void hal_timer_stop(void);

/**
 * @brief Reset timer counter
 */
void hal_timer_reset(void);

/**
 * @brief Set timeout value
 * @param timeout_ms Timeout in milliseconds
 */
void hal_timer_set_timeout(uint32_t timeout_ms);

/**
 * @brief Set timeout callback
 * @param callback Callback function
 */
void hal_timer_set_callback(void (*callback)(void));

/**
 * @brief Check if timer expired (polling mode)
 * @return true if expired, false otherwise
 */
bool hal_timer_is_expired(void);

/**
 * @brief Check if timer is running
 * @return true if running, false otherwise
 */
bool hal_timer_is_running(void);

/**
 * @brief Get current counter value
 * @return Counter value in timer ticks
 */
uint32_t hal_timer_get_counter(void);

/**
 * @brief Get elapsed time in ms
 * @return Elapsed time in milliseconds
 */
uint32_t hal_timer_get_elapsed_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIMER_H */
