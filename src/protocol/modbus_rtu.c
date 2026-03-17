/**
 * @file modbus_rtu.c
 * @brief Modbus RTU帧处理实现
 * @author Claude
 * @date 2026-03-17
 */

#include "modbus_rtu.h"
#include "modbus_crc.h"
#include <string.h>

/**
 * @brief 解析RTU帧
 */
int modbus_rtu_parse(const uint8_t *buffer, uint16_t len, mb_rtu_frame_t *frame)
{
    if (buffer == NULL || frame == NULL) {
        return -1;
    }

    /* 检查最小帧长度（地址+功能码+CRC16 = 4字节） */
    if (len < MB_RTU_FRAME_MIN_SIZE) {
        return -1;
    }

    /* 验证CRC */
    if (modbus_crc_verify(buffer, len) != 0) {
        return -1;
    }

    /* 解析帧 */
    frame->slave_addr = buffer[0];
    frame->function_code = buffer[1];
    frame->data = (uint8_t *)&buffer[2];
    frame->data_len = len - 4;  /* 减去地址(1) + 功能码(1) + CRC(2) */

    return 0;
}

/**
 * @brief 构建RTU响应帧
 */
uint16_t modbus_rtu_build_response(
    uint8_t slave_addr,
    uint8_t function_code,
    const uint8_t *data,
    uint16_t data_len,
    uint8_t *out_buf
)
{
    uint16_t idx = 0;

    if (out_buf == NULL) {
        return 0;
    }

    /* 从站地址 */
    out_buf[idx++] = slave_addr;

    /* 功能码 */
    out_buf[idx++] = function_code;

    /* 数据 */
    if (data != NULL && data_len > 0) {
        memcpy(&out_buf[idx], data, data_len);
        idx += data_len;
    }

    /* 计算并追加CRC */
    modbus_crc_append(out_buf, idx);
    idx += 2;

    return idx;
}

/**
 * @brief 构建异常响应帧
 */
uint16_t modbus_rtu_build_exception(
    uint8_t slave_addr,
    uint8_t function_code,
    mb_exception_t exception_code,
    uint8_t *out_buf
)
{
    uint16_t idx = 0;

    if (out_buf == NULL) {
        return 0;
    }

    /* 从站地址 */
    out_buf[idx++] = slave_addr;

    /* 功能码（设置最高位表示异常） */
    out_buf[idx++] = function_code | MB_EXCEPTION_BIT;

    /* 异常码 */
    out_buf[idx++] = (uint8_t)exception_code;

    /* 计算并追加CRC */
    modbus_crc_append(out_buf, idx);
    idx += 2;

    return idx;
}

/**
 * @brief 检查帧是否为广播帧
 */
bool modbus_rtu_is_broadcast(uint8_t slave_addr)
{
    return (slave_addr == MB_SLAVE_ADDR_BROADCAST);
}

/**
 * @brief 验证帧最小长度
 */
bool modbus_rtu_validate_min_length(uint16_t len)
{
    return (len >= MB_RTU_FRAME_MIN_SIZE);
}
