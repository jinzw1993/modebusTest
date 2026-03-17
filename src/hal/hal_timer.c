/**
 * @file hal_timer.c
 * @brief Timer Hardware Abstraction Layer Implementation
 * @author Claude
 * @date 2026-03-17
 */

#include "hal_timer.h"
#include <stddef.h>

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

static const hal_timer_t *s_hal_timer = NULL;

/* ============================================================================
 * API Implementation
 * ============================================================================ */

int hal_timer_register(const hal_timer_t *hal)
{
    if (hal == NULL) {
        return -1;
    }

    s_hal_timer = hal;
    return 0;
}

int hal_timer_init(uint32_t timeout_ms)
{
    if (s_hal_timer == NULL || s_hal_timer->init == NULL) {
        return -1;
    }

    return s_hal_timer->init(timeout_ms);
}

void hal_timer_deinit(void)
{
    if (s_hal_timer != NULL && s_hal_timer->deinit != NULL) {
        s_hal_timer->deinit();
    }
}

void hal_timer_start(void)
{
    if (s_hal_timer != NULL && s_hal_timer->start != NULL) {
        s_hal_timer->start();
    }
}

void hal_timer_stop(void)
{
    if (s_hal_timer != NULL && s_hal_timer->stop != NULL) {
        s_hal_timer->stop();
    }
}

void hal_timer_reset(void)
{
    if (s_hal_timer != NULL && s_hal_timer->reset != NULL) {
        s_hal_timer->reset();
    }
}

void hal_timer_set_timeout(uint32_t timeout_ms)
{
    if (s_hal_timer != NULL && s_hal_timer->set_timeout != NULL) {
        s_hal_timer->set_timeout(timeout_ms);
    }
}

void hal_timer_set_callback(void (*callback)(void))
{
    if (s_hal_timer != NULL && s_hal_timer->set_callback != NULL) {
        s_hal_timer->set_callback(callback);
    }
}

bool hal_timer_is_expired(void)
{
    if (s_hal_timer != NULL && s_hal_timer->is_expired != NULL) {
        return s_hal_timer->is_expired();
    }

    return false;
}

bool hal_timer_is_running(void)
{
    if (s_hal_timer != NULL && s_hal_timer->is_running != NULL) {
        return s_hal_timer->is_running();
    }

    return false;
}

uint32_t hal_timer_get_counter(void)
{
    if (s_hal_timer != NULL && s_hal_timer->get_counter != NULL) {
        return s_hal_timer->get_counter();
    }

    return 0;
}

uint32_t hal_timer_get_elapsed_ms(void)
{
    if (s_hal_timer != NULL && s_hal_timer->get_elapsed_ms != NULL) {
        return s_hal_timer->get_elapsed_ms();
    }

    return 0;
}
