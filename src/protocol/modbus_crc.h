/**
 * @file modbus_crc.h
 * @brief Modbus CRC16校验接口
 * @author Claude
 * @date 2026-03-17
 */

#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 计算数据的CRC16校验值（Modbus标准）
 * @param data 数据指针
 * @param len 数据长度
 * @return CRC16校验值
 */
uint16_t modbus_crc16(const uint8_t *data, uint16_t len);

/**
 * @brief 验证数据的CRC16校验值
 * @param data 数据指针（包含CRC，CRC在最后2字节）
 * @param len 数据总长度（包含2字节CRC）
 * @return 0:校验成功, -1:校验失败
 */
int modbus_crc_verify(const uint8_t *data, uint16_t len);

/**
 * @brief 向缓冲区追加CRC16校验值
 * @param data 数据缓冲区
 * @param len 数据长度（不含CRC）
 * @note CRC值将写入data[len]和data[len+1]
 */
void modbus_crc_append(uint8_t *data, uint16_t len);

/**
 * @brief 获取CRC16的高字节和低字节
 * @param crc CRC16值
 * @param high 存储高字节的指针
 * @param low 存储低字节的指针
 */
void modbus_crc_get_bytes(uint16_t crc, uint8_t *high, uint8_t *low);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_CRC_H */
