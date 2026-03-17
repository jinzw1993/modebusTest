/**
 * @file port_timer_at32.c
 * @brief AT32F432定时器移植实现
 * @author Claude
 * @date 2026-03-17
 *
 * @note 这是AT32F432标准外设库的移植实现
 *       使用TMR3作为Modbus T3.5定时器
 */

#include "../hal/hal_timer.h"
#include "at32f423.h"
#include "at32f423_tmr.h"
#include "at32f423_crm.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * 移植配置
 * ============================================================================ */

/* 使用TMR3作为T3.5定时器 */
#define PORT_TMR                TMR3
#define PORT_TMR_IRQn           TMR3_GLOBAL_IRQn

/* 时钟配置 */
#define PORT_TMR_CLK            CRM_TMR3_PERIPH_CLOCK

/* ============================================================================
 * 内部变量
 * ============================================================================ */

/* 超时回调函数指针 */
static void (*s_timeout_callback)(void) = NULL;

/* 当前超时时间（毫秒） */
static uint32_t s_timeout_ms = 10;

/* ============================================================================
 * AT32中断服务函数
 * ============================================================================ */

/**
 * @brief TMR3中断服务函数
 */
void TMR3_GLOBAL_IRQHandler(void)
{
    if (tmr_flag_get(PORT_TMR, TMR_OVF_FLAG) != RESET) {
        /* 清除中断标志 */
        tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);

        /* 停止定时器 */
        tmr_counter_enable(PORT_TMR, FALSE);

        /* 调用超时回调 */
        if (s_timeout_callback != NULL) {
            s_timeout_callback();
        }
    }
}

/* ============================================================================
 * 定时器HAL接口实现
 * ============================================================================ */

/**
 * @brief 初始化定时器
 */
static int port_timer_init(uint32_t timeout_ms)
{
    tmr_base_init_type tmr_init_struct;
    uint32_t prescaler;
    uint32_t period;

    /* 使能时钟 */
    crm_periph_clock_enable(PORT_TMR_CLK, TRUE);

    /* 复位定时器 */
    tmr_reset(PORT_TMR);

    /*
     * 计算预分频和周期值
     * 假设系统时钟为144MHz (APB1时钟)
     * 目标：1ms定时精度
     * 预分频 = 14400 - 1 (144MHz / 14400 = 10kHz, 即0.1ms)
     * 周期 = timeout_ms * 10 - 1
     */
    prescaler = 14400 - 1;
    period = timeout_ms * 10 - 1;

    /* 配置定时器基础参数 */
    tmr_init_struct.tmr_period = period;
    tmr_init_struct.tmr_div = prescaler;
    tmr_init_struct.tmr_cnt_dir = TMR_CNT_DIR_UP;
    tmr_init_struct.tmr_clk_div = TMR_CLK_DIV1;
    tmr_init_struct.tmr_rep_cnt = 0;

    tmr_base_init(PORT_TMR, &tmr_init_struct);

    /* 清除中断标志 */
    tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);

    /* 配置中断优先级 */
    nvic_irq_enable(PORT_TMR_IRQn, 0, 0);

    /* 暂不使能定时器 */
    tmr_counter_enable(PORT_TMR, FALSE);

    /* 暂不使能更新中断 */
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, FALSE);

    s_timeout_ms = timeout_ms;

    return 0;
}

/**
 * @brief 反初始化定时器
 */
static void port_timer_deinit(void)
{
    tmr_reset(PORT_TMR);
    tmr_counter_enable(PORT_TMR, FALSE);
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, FALSE);
    nvic_irq_enable(PORT_TMR_IRQn, FALSE);
}

/**
 * @brief 启动定时器
 */
static void port_timer_start(void)
{
    /* 重置计数器 */
    tmr_counter_value_set(PORT_TMR, 0);

    /* 清除中断标志 */
    tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);

    /* 使能更新中断 */
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, TRUE);

    /* 启动定时器 */
    tmr_counter_enable(PORT_TMR, TRUE);
}

/**
 * @brief 停止定时器
 */
static void port_timer_stop(void)
{
    tmr_counter_enable(PORT_TMR, FALSE);
    tmr_interrupt_enable(PORT_TMR, TMR_OVF_INT, FALSE);
}

/**
 * @brief 重置定时器
 */
static void port_timer_reset(void)
{
    /* 重置计数器 */
    tmr_counter_value_set(PORT_TMR, 0);

    /* 清除中断标志 */
    tmr_flag_clear(PORT_TMR, TMR_OVF_FLAG);
}

/**
 * @brief 设置超时时间
 */
static void port_timer_set_timeout(uint32_t timeout_ms)
{
    uint32_t period;

    s_timeout_ms = timeout_ms;

    /* 计算周期值 */
    period = timeout_ms * 10 - 1;

    /* 更新自动重装载值 */
    tmr_period_value_set(PORT_TMR, period);
}

/**
 * @brief 设置超时回调函数
 */
static void port_timer_set_callback(void (*callback)(void))
{
    s_timeout_callback = callback;
}

/**
 * @brief 检查定时器是否已超时（轮询模式）
 */
static bool port_timer_is_expired(void)
{
    return (tmr_flag_get(PORT_TMR, TMR_OVF_FLAG) != RESET);
}

/* ============================================================================
 * HAL接口注册
 * ============================================================================ */

/* AT32定时器HAL接口 */
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
 * @brief 注册AT32定时器HAL接口
 */
void port_timer_at32_register(void)
{
    hal_timer_register(&hal_timer_at32);
}
