/**
 * @file port_timer_at32.c
 * @brief AT32F423 timer port implementation
 * @author Claude
 * @date 2026-03-17
 */

#include "../hal/hal_timer.h"
#include "at32f423.h"
#include "at32f423_tmr.h"
#include "at32f423_crm.h"
#include <stddef.h>

/* ============================================================================
 * Port Configuration
 * ============================================================================ */

/* Use TMR3 as T3.5 timer */
#define PORT_TMR                TMR3
#define PORT_TMR_IRQn           TMR3_GLOBAL_IRQn

/* Clock configuration */
#define PORT_TMR_CLK            CRM_TMR3_PERIPH_CLOCK

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

/* Timeout callback function pointer */
static void (*s_timeout_callback)(void) = NULL;

/* ============================================================================
 * AT32 Interrupt Service Function
 * ============================================================================ */

/**
 * @brief TMR3 interrupt handler
 */
void TMR3_GLOBAL_IRQHandler(void)
{
    if (tmr_flag_get(PORT_TMR, TMR_OVF_FLAG) != RESET) {
        /* Clear interrupt flag */
        tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);

        /* Stop timer */
        tmr_counter_enable(PORT_TMR, FALSE);

        /* Call timeout callback */
        if (s_timeout_callback != NULL) {
            s_timeout_callback();
        }
    }
}

/* ============================================================================
 * Timer HAL Interface Implementation
 * ============================================================================ */

/**
 * @brief Initialize timer
 */
static int port_timer_init(uint32_t timeout_ms)
{
    uint32_t prescaler;
    uint32_t period;

    /* Enable clock */
    crm_periph_clock_enable(PORT_TMR_CLK, TRUE);

    /* Reset timer */
    tmr_reset(PORT_TMR);

    /*
     * Calculate prescaler and period
     * Assume system clock is 144MHz (APB1 clock)
     * Target: 1ms timer resolution
     * Prescaler = 14400 - 1 (144MHz / 14400 = 10kHz, 0.1ms per tick)
     * Period = timeout_ms * 10 - 1
     */
    prescaler = 14400 - 1;
    period = timeout_ms * 10 - 1;

    /* Configure timer base */
    tmr_base_init(PORT_TMR, period, prescaler);
    tmr_clock_source_div_set(PORT_TMR, TMR_CLOCK_DIV1);
    tmr_cnt_dir_set(PORT_TMR, TMR_COUNT_UP);

    /* Clear interrupt flag */
    tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);

    /* Configure interrupt priority */
    nvic_irq_enable(PORT_TMR_IRQn, 0, 0);

    /* Don't enable timer yet */
    tmr_counter_enable(PORT_TMR, FALSE);

    /* Don't enable update interrupt yet */
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, FALSE);

    return 0;
}

/**
 * @brief Deinitialize timer
 */
static void port_timer_deinit(void)
{
    tmr_reset(PORT_TMR);
    tmr_counter_enable(PORT_TMR, FALSE);
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, FALSE);
    nvic_irq_enable(PORT_TMR_IRQn, FALSE);
}

/**
 * @brief Start timer
 */
static void port_timer_start(void)
{
    /* Reset counter */
    tmr_counter_value_set(PORT_TMR, 0);

    /* Clear interrupt flag */
    tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);

    /* Enable update interrupt */
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, TRUE);

    /* Start timer */
    tmr_counter_enable(PORT_TMR, TRUE);
}

/**
 * @brief Stop timer
 */
static void port_timer_stop(void)
{
    tmr_counter_enable(PORT_TMR, FALSE);
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, FALSE);
}

/**
 * @brief Reset timer
 */
static void port_timer_reset(void)
{
    /* Reset counter */
    tmr_counter_value_set(PORT_TMR, 0);

    /* Clear interrupt flag */
    tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);
}

/**
 * @brief Set timeout value
 */
static void port_timer_set_timeout(uint32_t timeout_ms)
{
    uint32_t period;

    /* Calculate period */
    period = timeout_ms * 10 - 1;

    /* Update auto-reload value */
    tmr_period_value_set(PORT_TMR, period);
}

/**
 * @brief Set timeout callback
 */
static void port_timer_set_callback(void (*callback)(void))
{
    s_timeout_callback = callback;
}

/**
 * @brief Check if timer expired (polling mode)
 */
static bool port_timer_is_expired(void)
{
    return (tmr_flag_get(PORT_TMR, TMR_OVF_FLAG) != RESET);
}

/* ============================================================================
 * HAL Interface Registration
 * ============================================================================ */

/* AT32 timer HAL interface */
const hal_timer_t hal_timer_at32 = {
    .init = port_timer_init,
    .deinit = port_timer_deinit,
    .start = port_timer_start,
    .stop = port_timer_stop,
    .reset = port_timer_reset,
    .set_timeout = port_timer_set_timeout,
    .set_callback = port_timer_set_callback,
    .is_expired = port_timer_is_expired
};

/**
 * @brief Register AT32 timer HAL interface
 */
void port_timer_at32_register(void)
{
    hal_timer_register(&hal_timer_at32);
}
