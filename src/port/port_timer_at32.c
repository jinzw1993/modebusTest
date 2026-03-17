/**
 * @file port_timer_at32.c
 * @brief AT32F423 Timer driver for Modbus RTU T3.5 detection
 * @author Claude
 * @date 2026-03-17
 */

#include "../hal/hal_timer.h"
#include <stddef.h>
#include "at32f423.h"
#include "at32f423_tmr.h"
#include "at32f423_crm.h"
#include "at32f423_misc.h"

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define PORT_TMRx               TMR3
#define PORT_TMRx_IRQn          TMR3_GLOBAL_IRQn
#define PORT_TMRx_CLK           CRM_TMR3_PERIPH_CLOCK

#define PORT_TMRx_PREEMPT_PRI   1
#define PORT_TMRx_SUB_PRI       0

/* Clock: 144MHz, Prescaler: 14400-1 => 10kHz (0.1ms) */
#define TMR_PRESCALER_1MS       14400
#define TMR_TICKS_PER_MS        10

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

static void (*s_timeout_callback)(void) = NULL;
static volatile uint32_t s_timeout_ms = 10;
static volatile uint8_t s_is_running = 0;

/* ============================================================================
 * Interrupt Handler
 * ============================================================================ */

void TMR3_GLOBAL_IRQHandler(void)
{
    if (tmr_flag_get(PORT_TMRx, TMR_OVF_FLAG) != RESET) {
        tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);

        tmr_counter_enable(PORT_TMRx, FALSE);
        tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);
        s_is_running = 0;

        if (s_timeout_callback != NULL) {
            s_timeout_callback();
        }
    }
}

/* ============================================================================
 * HAL Interface Implementation
 * ============================================================================ */

static int port_timer_init(uint32_t timeout_ms)
{
    uint32_t prescaler;
    uint32_t period;

    crm_periph_clock_enable(PORT_TMRx_CLK, TRUE);
    tmr_reset(PORT_TMRx);

    prescaler = TMR_PRESCALER_1MS - 1;
    period = timeout_ms * TMR_TICKS_PER_MS - 1;

    tmr_base_init(PORT_TMRx, period, prescaler);
    tmr_clock_source_div_set(PORT_TMRx, TMR_CLOCK_DIV1);
    tmr_cnt_dir_set(PORT_TMRx, TMR_COUNT_UP);
    tmr_period_buffer_enable(PORT_TMRx, TRUE);
    tmr_internal_clock_set(PORT_TMRx);

    tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);

    nvic_irq_enable(PORT_TMRx_IRQn, PORT_TMRx_PREEMPT_PRI, PORT_TMRx_SUB_PRI);

    tmr_counter_enable(PORT_TMRx, FALSE);
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);

    s_timeout_ms = timeout_ms;
    s_is_running = 0;

    return 0;
}

static void port_timer_deinit(void)
{
    tmr_counter_enable(PORT_TMRx, FALSE);
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);
    tmr_reset(PORT_TMRx);
    nvic_irq_disable(PORT_TMRx_IRQn);
    s_is_running = 0;
}

static void port_timer_start(void)
{
    tmr_counter_value_set(PORT_TMRx, 0);
    tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, TRUE);
    tmr_counter_enable(PORT_TMRx, TRUE);
    s_is_running = 1;
}

static void port_timer_stop(void)
{
    tmr_counter_enable(PORT_TMRx, FALSE);
    tmr_interrupt_enable(PORT_TMRx, TMR_OVF_INT, FALSE);
    s_is_running = 0;
}

static void port_timer_reset(void)
{
    tmr_counter_value_set(PORT_TMRx, 0);
    tmr_flag_clear(PORT_TMRx, TMR_OVF_FLAG);
}

static void port_timer_set_timeout(uint32_t timeout_ms)
{
    uint32_t period = timeout_ms * TMR_TICKS_PER_MS - 1;
    tmr_period_value_set(PORT_TMRx, period);
    s_timeout_ms = timeout_ms;
}

static void port_timer_set_callback(void (*callback)(void))
{
    s_timeout_callback = callback;
}

static bool port_timer_is_expired(void)
{
    return (tmr_flag_get(PORT_TMRx, TMR_OVF_FLAG) != RESET);
}

static bool port_timer_is_running(void)
{
    return (s_is_running != 0);
}

/* ============================================================================
 * HAL Instance
 * ============================================================================ */

static const hal_timer_t s_hal_timer_at32 = {
    .init = port_timer_init,
    .deinit = port_timer_deinit,
    .start = port_timer_start,
    .stop = port_timer_stop,
    .reset = port_timer_reset,
    .set_timeout = port_timer_set_timeout,
    .set_callback = port_timer_set_callback,
    .is_expired = port_timer_is_expired,
    .is_running = port_timer_is_running
};

/* ============================================================================
 * Registration
 * ============================================================================ */

void port_timer_at32_register(void)
{
    hal_timer_register(&s_hal_timer_at32);
}
