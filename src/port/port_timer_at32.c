/**
 * @file port_timer_at32.c
 * @brief AT32F423 Timer driver for Modbus RTU T3.5 detection
 * @author Claude
 * @date 2026-03-17
 *
 * @note Features:
 *       - Configurable timeout
 *       - One-shot mode for T3.5 detection
 *       - Low interrupt latency
 *       - Automatic reload prevention
 */

#include "../hal/hal_timer.h"
#include "at32f423.h"
#include "at32f423_tmr.h"
#include "at32f423_crm.h"
#include "at32f423_misc.h"
#include <stddef.h>

/* ============================================================================
 * Configuration
 * ============================================================================ */

/* Timer selection - TMR3 (general purpose timer) */
#define PORT_TMRx               TMR3
#define PORT_TMRx_IRQn          TMR3_GLOBAL_IRQn
#define PORT_TMRx_CLK           CRM_TMR3_PERIPH_CLOCK

/* Interrupt priority */
#define PORT_TMRx_PREEMPT_PRI   1
#define PORT_TMRx_SUB_PRI       0

/* Clock configuration
 * AT32F423 APB1 max clock: 144MHz
 * Timer clock = APB1 * 1 = 144MHz (when APB1 prescaler != 1)
 *
 * For 1ms resolution:
 * Prescaler = 14400 - 1  =>  144MHz / 14400 = 10kHz (0.1ms per tick)
 * Period = timeout_ms * 10 - 1
 */
#define TMR_PRESCALER_1MS       14400
#define TMR_TICKS_PER_MS        10

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

/* Timeout callback */
static void (*s_timeout_callback)(void) = NULL;

/* Current timeout in ms */
static volatile uint32_t s_timeout_ms = 10;

/* Running flag */
static volatile uint8_t s_is_running = 0;

/* ============================================================================
 * Interrupt Handler
 * ============================================================================ */

/**
 * @brief TMR3 global interrupt handler
 */
void TMR3_GLOBAL_IRQHandler(void)
{
    if (tmr_flag_get(PORT_TMRx, TMR_OVF_FLAG) != RESET) {
        /* Clear interrupt flag first */
        tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);

        /* Stop timer (one-shot mode) */
        tmr_counter_enable(PORT_TMRx, FALSE);
        tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);
        s_is_running = 0;

        /* Call user callback */
        if (s_timeout_callback != NULL) {
            s_timeout_callback();
        }
    }
}

/* ============================================================================
 * HAL Interface Implementation
 * ============================================================================ */

/**
 * @brief Initialize timer
 * @param timeout_ms Timeout value in milliseconds
 * @return 0 on success, -1 on error
 */
static int port_timer_init(uint32_t timeout_ms)
{
    tmr_clock_division_type clock_div;
    uint32_t prescaler;
    uint32_t period;

    /* Enable timer clock */
    crm_periph_clock_enable(PORT_TMRx_CLK, TRUE);

    /* Reset timer to default state */
    tmr_reset(PORT_TMRx);

    /* Calculate prescaler and period
     * Clock: 144MHz
     * Prescaler: 14400 - 1 => 10kHz (0.1ms resolution)
     * Period: timeout_ms * 10 - 1
     */
    prescaler = TMR_PRESCALER_1MS - 1;
    period = timeout_ms * TMR_TICKS_PER_MS - 1;

    /* Configure timer base */
    tmr_base_init(PORT_TMRx, period, prescaler);

    /* Configure counter direction: up counting */
    tmr_cnt_dir_set(PORT_TMRx, TMR_COUNT_UP);

    /* Configure clock division: DIV1 */
    tmr_clock_source_div_set(PORT_TMRx, TMR_CLOCK_DIV1);

    /* Configure auto-reload buffer: enable */
    tmr_period_buffer_enable(PORT_TMRx, TRUE);

    /* Disable repetition counter (not used) */
    tmr_repetition_counter_set(PORT_TMRx, 0);

    /* Set internal clock source */
    tmr_internal_clock_set(PORT_TMRx);

    /* Clear any pending flags */
    tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);

    /* Configure NVIC */
    nvic_irq_enable(PORT_TMRx_IRQn, PORT_TMRx_PREEMPT_PRI, PORT_TMRx_SUB_PRI);

    /* Don't enable timer yet */
    tmr_counter_enable(PORT_TMRx, FALSE);

    /* Don't enable interrupt yet */
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);

    /* Store timeout value */
    s_timeout_ms = timeout_ms;
    s_is_running = 0;

    return 0;
}

/**
 * @brief Deinitialize timer
 */
static void port_timer_deinit(void)
{
    /* Stop timer */
    tmr_counter_enable(PORT_TMRx, FALSE);
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);

    /* Clear flag */
    tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);

    /* Disable NVIC */
    nvic_irq_disable(PORT_TMRx_IRQn);

    /* Reset timer */
    tmr_reset(PORT_TMRx);

    s_is_running = 0;
}

/**
 * @brief Start timer (one-shot)
 */
static void port_timer_start(void)
{
    /* Reset counter */
    tmr_counter_value_set(PORT_TMRx, 0);

    /* Clear overflow flag */
    tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);

    /* Enable overflow interrupt */
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, TRUE);

    /* Start counter */
    tmr_counter_enable(PORT_TMRx, TRUE);

    s_is_running = 1;
}

/**
 * @brief Stop timer
 */
static void port_timer_stop(void)
{
    tmr_counter_enable(PORT_TMRx, FALSE);
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);
    s_is_running = 0;
}

/**
 * @brief Reset timer counter (restart from 0)
 */
static void port_timer_reset(void)
{
    /* Reset counter */
    tmr_counter_value_set(PORT_TMRx, 0);

    /* Clear overflow flag */
    tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);
}

/**
 * @brief Set timeout value
 * @param timeout_ms Timeout in milliseconds
 */
static void port_timer_set_timeout(uint32_t timeout_ms)
{
    uint32_t period;

    /* Calculate period */
    period = timeout_ms * TMR_TICKS_PER_MS - 1;

    /* Update period register */
    tmr_period_value_set(PORT_TMRx, period);

    /* Store timeout value */
    s_timeout_ms = timeout_ms;
}

/**
 * @brief Set timeout callback
 * @param callback Callback function
 */
static void port_timer_set_callback(void (*callback)(void))
{
    s_timeout_callback = callback;
}

/**
 * @brief Check if timer expired (polling mode)
 * @return true if expired, false otherwise
 */
static bool port_timer_is_expired(void)
{
    return (tmr_flag_get(PORT_TMRx, TMR_OVF_FLAG) != RESET);
}

/**
 * @brief Check if timer is running
 * @return true if running, false if stopped
 */
static bool port_timer_is_running(void)
{
    return (s_is_running != 0);
}

/**
 * @brief Get current counter value
 * @return Counter value in timer ticks
 */
static uint32_t port_timer_get_counter(void)
{
    return tmr_counter_value_get(PORT_TMRx);
}

/**
 * @brief Get elapsed time in ms
 * @return Elapsed time in milliseconds
 */
static uint32_t port_timer_get_elapsed_ms(void)
{
    return tmr_counter_value_get(PORT_TMRx) / TMR_TICKS_PER_MS;
}

/* ============================================================================
 * HAL Registration
 * ============================================================================ */

const hal_timer_t hal_timer_at32 = {
    .init = port_timer_init,
    .deinit = port_timer_deinit,
    .start = port_timer_start,
    .stop = port_timer_stop,
    .reset = port_timer_reset,
    .set_timeout = port_timer_set_timeout,
    .set_callback = port_timer_set_callback,
    .is_expired = port_timer_is_expired,
    .is_running = port_timer_is_running,
    .get_counter = port_timer_get_counter,
    .get_elapsed_ms = port_timer_get_elapsed_ms
};

void port_timer_at32_register(void)
{
    hal_timer_register(&hal_timer_at32);
}
