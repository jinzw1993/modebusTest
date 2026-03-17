/**
 * @file hal_timer.c
 * @brief 定时器硬件抽象层实现
 * @author Claude
 * @date 2026-03-17
 */

#include "hal_timer.h"
#include <stddef.h>

/* 当前注册的HAL接口 */
static const hal_timer_t *s_hal_timer = NULL;

/**
 * @brief 注册定时器HAL接口
 */
int hal_timer_register(const hal_timer_t *hal)
{
    if (hal == NULL) {
        return -1;
    }

    s_hal_timer = hal;
    return 0;
}

/**
 * @brief 初始化定时器
 */
int hal_timer_init(uint32_t timeout_ms)
{
    if (s_hal_timer == NULL || s_hal_timer->init == NULL) {
        return -1;
    }

    return s_hal_timer->init(timeout_ms);
}

/**
 * @brief 反初始化定时器
 */
void hal_timer_deinit(void)
{
    if (s_hal_timer != NULL && s_hal_timer->deinit != NULL) {
        s_hal_timer->deinit();
    }
}

/**
 * @brief 启动定时器
 */
void hal_timer_start(void)
{
    if (s_hal_timer != NULL && s_hal_timer->start != NULL) {
        s_hal_timer->start();
    }
}

/**
 * @brief 停止定时器
 */
void hal_timer_stop(void)
{
    if (s_hal_timer != NULL && s_hal_timer->stop != NULL) {
        s_hal_timer->stop();
    }
}

/**
 * @brief 重置定时器
 */
void hal_timer_reset(void)
{
    if (s_hal_timer != NULL && s_hal_timer->reset != NULL) {
        s_hal_timer->reset();
    }
}

/**
 * @brief 设置超时时间
 */
void hal_timer_set_timeout(uint32_t timeout_ms)
{
    if (s_hal_timer != NULL && s_hal_timer->set_timeout != NULL) {
        s_hal_timer->set_timeout(timeout_ms);
    }
}

/**
 * @brief 设置超时回调函数
 */
void hal_timer_set_callback(void (*callback)(void))
{
    if (s_hal_timer != NULL && s_hal_timer->set_callback != NULL) {
        s_hal_timer->set_callback(callback);
    }
}

/**
 * @brief 检查定时器是否已超时
 */
bool hal_timer_is_expired(void)
{
    if (s_hal_timer != NULL && s_hal_timer->is_expired != NULL) {
        return s_hal_timer->is_expired();
    }

    return false;
}
