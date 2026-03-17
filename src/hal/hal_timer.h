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
typedef struct hal_timer {
    int  (*init)(uint32_t timeout_ms);
    void (*deinit)(void);
    void (*start)(void);
    void (*stop)(void);
    void (*reset)(void);
    void (*set_timeout)(uint32_t timeout_ms);
    void (*set_callback)(void (*callback)(void));
    bool (*is_expired)(void);
    bool (*is_running)(void);
} hal_timer_t;

/* ============================================================================
 * Register Function
 * ============================================================================ */

int hal_timer_register(const hal_timer_t *hal);

/* ============================================================================
 * API Functions
 * ============================================================================ */

int  hal_timer_init(uint32_t timeout_ms);
void hal_timer_deinit(void);
void hal_timer_start(void);
void hal_timer_stop(void);
void hal_timer_reset(void);
void hal_timer_set_timeout(uint32_t timeout_ms);
void hal_timer_set_callback(void (*callback)(void));
bool hal_timer_is_expired(void);
bool hal_timer_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIMER_H */
