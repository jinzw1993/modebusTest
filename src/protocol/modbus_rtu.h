/**
 * @file modbus_rtu.h
 * @brief Modbus RTU帧处理接口
 * @author Claude
 * @date 2026-03-17
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <stdint.h>
#include <stdbool.h>
#include "modbus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * RTU帧结构定义
 * ============================================================================ */

/**
 * @brief RTU帧结构（解析后的帧）
 */
typedef struct {
    uint8_t  slave_addr;        /* 从站地址：1字节 */
    uint8_t  function_code;     /* 功能码：1字节 */
    uint8_t  *data;             /* 数据指针：最多252字节 */
    uint16_t data_len;          /* 数据长度 */
} mb_rtu_frame_t;

/**
 * @brief Modbus异常码类型（用于构建异常响应）
 */
typedef mb_exception_t mb_exception_code_t;

/* ============================================================================
 * RTU帧处理函数
 * ============================================================================ */

/**
 * @brief 解析RTU帧
 * @param buffer 原始帧数据（包含CRC）
 * @param len 帧长度
 * @param frame 输出的帧结构
 * @return 0:成功, <0:失败
 */
int modbus_rtu_parse(const uint8_t *buffer, uint16_t len, mb_rtu_frame_t *frame);

/**
 * @brief 构建RTU响应帧
 * @param slave_addr 从站地址
 * @param function_code 功能码
 * @param data 数据指针
 * @param data_len 数据长度
 * @param out_buf 输出缓冲区
 * @param out_len 输出长度
 * @return 帧总长度（包含CRC）
 */
uint16_t modbus_rtu_build_response(
    uint8_t slave_addr,
    uint8_t function_code,
    const uint8_t *data,
    uint16_t data_len,
    uint8_t *out_buf
);

/**
 * @brief 构建异常响应帧
 * @param slave_addr 从站地址
 * @param function_code 功能码
 * @param exception_code 异常码
 * @param out_buf 输出缓冲区
 * @return 帧总长度
 */
uint16_t modbus_rtu_build_exception(
    uint8_t slave_addr,
    uint8_t function_code,
    mb_exception_t exception_code,
    uint8_t *out_buf
);

/**
 * @brief 检查帧是否为广播帧
 * @param slave_addr 从站地址
 * @return true:广播帧, false:单播帧
 */
bool modbus_rtu_is_broadcast(uint8_t slave_addr);

/**
 * @brief 验证帧最小长度
 * @param len 帧长度
 * @return true:有效, false:无效
 */
bool modbus_rtu_validate_min_length(uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_RTU_H */
