/**
 * @file hal_timer.h
 * @brief 定时器硬件抽象层接口
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

/**
 * @brief 定时器操作接口结构
 */
typedef struct {
    /**
     * @brief 初始化定时器
     * @param timeout_ms 超时时间（毫秒）
     * @return 0:成功, <0:失败
     */
    int (*init)(uint32_t timeout_ms);

    /**
     * @brief 反初始化定时器
     */
    void (*deinit)(void);

    /**
     * @brief 启动定时器
     */
    void (*start)(void);

    /**
     * @brief 停止定时器
     */
    void (*stop)(void);

    /**
     * @brief 重置定时器（重新开始计时）
     */
    void (*reset)(void);

    /**
     * @brief 设置超时时间
     * @param timeout_ms 超时时间（毫秒）
     */
    void (*set_timeout)(uint32_t timeout_ms);

    /**
     * @brief 设置超时回调函数
     * @param callback 回调函数指针
     */
    void (*set_callback)(void (*callback)(void));

    /**
     * @brief 检查定时器是否已超时（轮询模式）
     * @return true:已超时, false:未超时
     */
    bool (*is_expired)(void);
} hal_timer_t;

/* ============================================================================
 * HAL接口函数
 * ============================================================================ */

/**
 * @brief 注册定时器HAL接口
 * @param hal 定时器操作接口结构指针
 * @return 0:成功, <0:失败
 */
int hal_timer_register(const hal_timer_t *hal);

/**
 * @brief 初始化定时器
 * @param timeout_ms 超时时间（毫秒）
 * @return 0:成功, <0:失败
 */
int hal_timer_init(uint32_t timeout_ms);

/**
 * @brief 反初始化定时器
 */
void hal_timer_deinit(void);

/**
 * @brief 启动定时器
 */
void hal_timer_start(void);

/**
 * @brief 停止定时器
 */
void hal_timer_stop(void);

/**
 * @brief 重置定时器
 */
void hal_timer_reset(void);

/**
 * @brief 设置超时时间
 */
void hal_timer_set_timeout(uint32_t timeout_ms);

/**
 * @brief 设置超时回调函数
 */
void hal_timer_set_callback(void (*callback)(void));

/**
 * @brief 检查定时器是否已超时
 */
bool hal_timer_is_expired(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIMER_H */
