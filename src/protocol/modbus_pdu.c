/**
 * @file modbus_pdu.c
 * @brief Modbus PDU功能码处理实现
 * @author Claude
 * @date 2026-03-17
 */

#include "modbus_pdu.h"
#include "modbus_data.h"
#include "modbus_config.h"
#include <string.h>

/* ============================================================================
 * 辅助宏定义
 * ============================================================================ */

/* 从请求中读取16位大端值 */
#define READ_UINT16_BE(buf, idx) \
    (((uint16_t)(buf)[idx] << 8) | (buf)[(idx) + 1])

/* 向响应中写入16位大端值 */
#define WRITE_UINT16_BE(buf, idx, val) \
    do { \
        (buf)[idx] = (uint8_t)((val) >> 8); \
        (buf)[idx + 1] = (uint8_t)((val) & 0xFF); \
    } while(0)

/* ============================================================================
 * PDU处理入口
 * ============================================================================ */

/**
 * @brief 处理PDU请求
 */
mb_pdu_result_t modbus_pdu_process(
    uint8_t function_code,
    const uint8_t *request_data,
    uint16_t request_len,
    uint8_t *response_data,
    uint16_t response_buf_size
)
{
    mb_pdu_result_t result = { MB_EX_ILLEGAL_FUNCTION, 0 };

    switch (function_code) {
        case MB_FC_READ_COILS:
            result.exception = modbus_pdu_read_coils(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        case MB_FC_READ_DISCRETE_INPUTS:
            result.exception = modbus_pdu_read_discrete_inputs(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        case MB_FC_READ_HOLDING_REGISTERS:
            result.exception = modbus_pdu_read_holding_registers(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        case MB_FC_READ_INPUT_REGISTERS:
            result.exception = modbus_pdu_read_input_registers(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        case MB_FC_WRITE_SINGLE_COIL:
            result.exception = modbus_pdu_write_single_coil(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        case MB_FC_WRITE_SINGLE_REGISTER:
            result.exception = modbus_pdu_write_single_register(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        case MB_FC_WRITE_MULTIPLE_COILS:
            result.exception = modbus_pdu_write_multiple_coils(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        case MB_FC_WRITE_MULTIPLE_REGISTERS:
            result.exception = modbus_pdu_write_multiple_registers(
                request_data, request_len,
                response_data, &result.response_len, response_buf_size
            );
            break;

        default:
            /* 不支持的功能码 */
            result.exception = MB_EX_ILLEGAL_FUNCTION;
            break;
    }

    return result;
}

/* ============================================================================
 * 功能码01: 读线圈
 * ============================================================================ */
mb_exception_t modbus_pdu_read_coils(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t start_addr;
    uint16_t count;
    uint16_t byte_count;
    uint16_t i;

    /* 检查请求长度 */
    if (request_len < 4) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    start_addr = READ_UINT16_BE(request, 0);
    count = READ_UINT16_BE(request, 2);

    /* 检查数量范围 */
    if (count < 1 || count > MB_MAX_COILS_PER_REQ) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查地址范围 */
    if (!modbus_data_check_coil_range(start_addr, count)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 计算响应字节数 */
    byte_count = (count + 7) / 8;

    /* 检查响应缓冲区大小 */
    if (buf_size < (1 + byte_count)) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    /* 构建响应：第一个字节是字节数 */
    response[0] = (uint8_t)byte_count;

    /* 读取线圈数据 */
    for (i = 0; i < byte_count; i++) {
        response[i + 1] = modbus_data_read_coils_byte(start_addr + i * 8, count);
        if (count >= 8) {
            count -= 8;
        } else {
            count = 0;
        }
    }

    *response_len = 1 + byte_count;
    return MB_EX_NONE;
}

/* ============================================================================
 * 功能码02: 读离散输入
 * ============================================================================ */
mb_exception_t modbus_pdu_read_discrete_inputs(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t start_addr;
    uint16_t count;
    uint16_t byte_count;
    uint16_t i;

    /* 检查请求长度 */
    if (request_len < 4) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    start_addr = READ_UINT16_BE(request, 0);
    count = READ_UINT16_BE(request, 2);

    /* 检查数量范围 */
    if (count < 1 || count > MB_MAX_COILS_PER_REQ) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查地址范围 */
    if (!modbus_data_check_discrete_input_range(start_addr, count)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 计算响应字节数 */
    byte_count = (count + 7) / 8;

    /* 检查响应缓冲区大小 */
    if (buf_size < (1 + byte_count)) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    /* 构建响应 */
    response[0] = (uint8_t)byte_count;

    /* 读取离散输入数据 */
    for (i = 0; i < byte_count; i++) {
        response[i + 1] = modbus_data_read_discrete_inputs_byte(start_addr + i * 8, count);
        if (count >= 8) {
            count -= 8;
        } else {
            count = 0;
        }
    }

    *response_len = 1 + byte_count;
    return MB_EX_NONE;
}

/* ============================================================================
 * 功能码03: 读保持寄存器
 * ============================================================================ */
mb_exception_t modbus_pdu_read_holding_registers(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t start_addr;
    uint16_t count;
    uint16_t byte_count;
    uint16_t i;
    uint16_t reg_value;

    /* 检查请求长度 */
    if (request_len < 4) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    start_addr = READ_UINT16_BE(request, 0);
    count = READ_UINT16_BE(request, 2);

    /* 检查数量范围 */
    if (count < 1 || count > MB_MAX_REGISTERS_PER_REQ) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查地址范围 */
    if (!modbus_data_check_holding_register_range(start_addr, count)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 计算响应字节数 */
    byte_count = count * 2;

    /* 检查响应缓冲区大小 */
    if (buf_size < (1 + byte_count)) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    /* 构建响应 */
    response[0] = (uint8_t)byte_count;

    /* 读取寄存器数据 */
    for (i = 0; i < count; i++) {
        reg_value = modbus_data_read_holding_register(start_addr + i);
        WRITE_UINT16_BE(response, 1 + i * 2, reg_value);
    }

    *response_len = 1 + byte_count;
    return MB_EX_NONE;
}

/* ============================================================================
 * 功能码04: 读输入寄存器
 * ============================================================================ */
mb_exception_t modbus_pdu_read_input_registers(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t start_addr;
    uint16_t count;
    uint16_t byte_count;
    uint16_t i;
    uint16_t reg_value;

    /* 检查请求长度 */
    if (request_len < 4) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    start_addr = READ_UINT16_BE(request, 0);
    count = READ_UINT16_BE(request, 2);

    /* 检查数量范围 */
    if (count < 1 || count > MB_MAX_REGISTERS_PER_REQ) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查地址范围 */
    if (!modbus_data_check_input_register_range(start_addr, count)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 计算响应字节数 */
    byte_count = count * 2;

    /* 检查响应缓冲区大小 */
    if (buf_size < (1 + byte_count)) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    /* 构建响应 */
    response[0] = (uint8_t)byte_count;

    /* 读取寄存器数据 */
    for (i = 0; i < count; i++) {
        reg_value = modbus_data_read_input_register(start_addr + i);
        WRITE_UINT16_BE(response, 1 + i * 2, reg_value);
    }

    *response_len = 1 + byte_count;
    return MB_EX_NONE;
}

/* ============================================================================
 * 功能码05: 写单个线圈
 * ============================================================================ */
mb_exception_t modbus_pdu_write_single_coil(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t addr;
    uint16_t value;

    /* 检查请求长度 */
    if (request_len < 4) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    addr = READ_UINT16_BE(request, 0);
    value = READ_UINT16_BE(request, 2);

    /* 检查地址范围 */
    if (!modbus_data_check_coil_range(addr, 1)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 检查值有效性 */
    if (value != 0x0000 && value != 0xFF00) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 写入线圈 */
    modbus_data_write_coil(addr, (value == 0xFF00) ? 1 : 0);

    /* 响应：回显请求 */
    if (buf_size < 4) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    memcpy(response, request, 4);
    *response_len = 4;
    return MB_EX_NONE;
}

/* ============================================================================
 * 功能码06: 写单个寄存器
 * ============================================================================ */
mb_exception_t modbus_pdu_write_single_register(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t addr;
    uint16_t value;

    /* 检查请求长度 */
    if (request_len < 4) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    addr = READ_UINT16_BE(request, 0);
    value = READ_UINT16_BE(request, 2);

    /* 检查地址范围 */
    if (!modbus_data_check_holding_register_range(addr, 1)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 写入寄存器 */
    modbus_data_write_holding_register(addr, value);

    /* 响应：回显请求 */
    if (buf_size < 4) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    memcpy(response, request, 4);
    *response_len = 4;
    return MB_EX_NONE;
}

/* ============================================================================
 * 功能码15: 写多个线圈
 * ============================================================================ */
mb_exception_t modbus_pdu_write_multiple_coils(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t start_addr;
    uint16_t count;
    uint8_t byte_count;
    uint16_t i;
    uint8_t bit_value;

    /* 检查请求长度 */
    if (request_len < 5) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    start_addr = READ_UINT16_BE(request, 0);
    count = READ_UINT16_BE(request, 2);
    byte_count = request[4];

    /* 检查数量范围 */
    if (count < 1 || count > MB_MAX_COILS_PER_REQ) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查字节数是否正确 */
    if (byte_count != (count + 7) / 8) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查请求长度是否足够 */
    if (request_len < (5 + byte_count)) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查地址范围 */
    if (!modbus_data_check_coil_range(start_addr, count)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 写入线圈数据 */
    for (i = 0; i < count; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_idx = i % 8;
        bit_value = (request[5 + byte_idx] >> bit_idx) & 0x01;
        modbus_data_write_coil(start_addr + i, bit_value);
    }

    /* 响应：起始地址 + 写入数量 */
    if (buf_size < 4) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    WRITE_UINT16_BE(response, 0, start_addr);
    WRITE_UINT16_BE(response, 2, count);
    *response_len = 4;
    return MB_EX_NONE;
}

/* ============================================================================
 * 功能码16: 写多个寄存器
 * ============================================================================ */
mb_exception_t modbus_pdu_write_multiple_registers(
    const uint8_t *request, uint16_t request_len,
    uint8_t *response, uint16_t *response_len, uint16_t buf_size
)
{
    uint16_t start_addr;
    uint16_t count;
    uint8_t byte_count;
    uint16_t i;
    uint16_t reg_value;

    /* 检查请求长度 */
    if (request_len < 5) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 解析参数 */
    start_addr = READ_UINT16_BE(request, 0);
    count = READ_UINT16_BE(request, 2);
    byte_count = request[4];

    /* 检查数量范围 */
    if (count < 1 || count > MB_MAX_REGISTERS_PER_REQ) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查字节数是否正确 */
    if (byte_count != count * 2) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查请求长度是否足够 */
    if (request_len < (5 + byte_count)) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }

    /* 检查地址范围 */
    if (!modbus_data_check_holding_register_range(start_addr, count)) {
        return MB_EX_ILLEGAL_DATA_ADDRESS;
    }

    /* 写入寄存器数据 */
    for (i = 0; i < count; i++) {
        reg_value = READ_UINT16_BE(request, 5 + i * 2);
        modbus_data_write_holding_register(start_addr + i, reg_value);
    }

    /* 响应：起始地址 + 写入数量 */
    if (buf_size < 4) {
        return MB_EX_SLAVE_DEVICE_FAILURE;
    }

    WRITE_UINT16_BE(response, 0, start_addr);
    WRITE_UINT16_BE(response, 2, count);
    *response_len = 4;
    return MB_EX_NONE;
}
