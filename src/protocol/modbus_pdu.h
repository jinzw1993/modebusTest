/**
 * @file modbus_pdu.h
 * @brief Modbus PDU功能码处理接口
 * @author Claude
 * @date 2026-03-17
 */

#ifndef MODBUS_PDU_H
#define MODBUS_PDU_H

#include <stdint.h>
#include "modbus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PDU处理结果
 * ============================================================================ */

/**
 * @brief PDU处理结果结构
 */
typedef struct {
    mb_exception_t exception;       /* 异常码，MB_EX_NONE表示成功 */
    uint16_t       response_len;    /* 响应数据长度 */
} mb_pdu_result_t;

/* ============================================================================
 * PDU处理函数
 * ============================================================================ */

/**
 * @brief 处理PDU请求
 * @param function_code 功能码
 * @param request_data 请求数据
 * @param request_len 请求长度
 * @param response_data 响应数据缓冲区
 * @param response_buf_size 响应缓冲区大小
 * @return 处理结果（包含异常码和响应长度）
 */
mb_pdu_result_t modbus_pdu_process(
    uint8_t function_code,
    const uint8_t *request_data,
    uint16_t request_len,
    uint8_t *response_data,
    uint16_t response_buf_size
);

/* ============================================================================
 * 各功能码处理函数（内部使用，但可单独测试）
 * ============================================================================ */

/**
 * @brief 处理读线圈请求（功能码01）
 */
mb_exception_t modbus_pdu_read_coils(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

/**
 * @brief 处理读离散输入请求（功能码02）
 */
mb_exception_t modbus_pdu_read_discrete_inputs(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

/**
 * @brief 处理读保持寄存器请求（功能码03）
 */
mb_exception_t modbus_pdu_read_holding_registers(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

/**
 * @brief 处理读输入寄存器请求（功能码04）
 */
mb_exception_t modbus_pdu_read_input_registers(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

/**
 * @brief 处理写单个线圈请求（功能码05）
 */
mb_exception_t modbus_pdu_write_single_coil(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

/**
 * @brief 处理写单个寄存器请求（功能码06）
 */
mb_exception_t modbus_pdu_write_single_register(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

/**
 * @brief 处理写多个线圈请求（功能码15）
 */
mb_exception_t modbus_pdu_write_multiple_coils(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

/**
 * @brief 处理写多个寄存器请求（功能码16）
 */
mb_exception_t modbus_pdu_write_multiple_registers(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_PDU_H */
