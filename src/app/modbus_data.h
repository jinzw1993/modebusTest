/**
 * @file modbus_data.h
 * @brief Modbus数据区管理接口
 * @author Claude
 * @date 2026-03-17
 */

#ifndef MODBUS_DATA_H
#define MODBUS_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "modbus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 数据区配置结构
 * ============================================================================ */

/**
 * @brief 数据区配置结构
 */
typedef struct {
    /* 线圈区（可读写位） */
    uint8_t *coils;
    uint16_t coils_size;
    mb_read_bit_cb_t read_coil_cb;
    mb_write_bit_cb_t write_coil_cb;

    /* 离散输入区（只读位） */
    uint8_t *discrete_inputs;
    uint16_t discrete_inputs_size;
    mb_read_bit_cb_t read_discrete_input_cb;

    /* 保持寄存器区（可读写16位） */
    uint16_t *holding_registers;
    uint16_t holding_registers_size;
    mb_read_reg_cb_t read_holding_reg_cb;
    mb_write_reg_cb_t write_holding_reg_cb;

    /* 输入寄存器区（只读16位） */
    uint16_t *input_registers;
    uint16_t input_registers_size;
    mb_read_reg_cb_t read_input_reg_cb;
} mb_data_config_t;

/* ============================================================================
 * 数据区初始化
 * ============================================================================ */

/**
 * @brief 初始化数据区
 * @param config 数据区配置
 * @return 0:成功, <0:失败
 */
int modbus_data_init(const mb_data_config_t *config);

/**
 * @brief 反初始化数据区
 */
void modbus_data_deinit(void);

/* ============================================================================
 * 线圈操作
 * ============================================================================ */

/**
 * @brief 读单个线圈
 * @param addr 线圈地址
 * @return 线圈值 (0或1)
 */
uint8_t modbus_data_read_coil(uint16_t addr);

/**
 * @brief 写单个线圈
 * @param addr 线圈地址
 * @param value 线圈值 (0或1)
 */
void modbus_data_write_coil(uint16_t addr, uint8_t value);

/**
 * @brief 读线圈字节（8个线圈）
 * @param addr 起始地址
 * @param remaining 剩余线圈数
 * @return 打包的字节
 */
uint8_t modbus_data_read_coils_byte(uint16_t addr, uint16_t remaining);

/**
 * @brief 检查线圈地址范围
 * @param start_addr 起始地址
 * @param count 数量
 * @return true:有效, false:无效
 */
bool modbus_data_check_coil_range(uint16_t start_addr, uint16_t count);

/* ============================================================================
 * 离散输入操作
 * ============================================================================ */

/**
 * @brief 读单个离散输入
 * @param addr 离散输入地址
 * @return 离散输入值 (0或1)
 */
uint8_t modbus_data_read_discrete_input(uint16_t addr);

/**
 * @brief 读离散输入字节（8个）
 * @param addr 起始地址
 * @param remaining 剩余数量
 * @return 打包的字节
 */
uint8_t modbus_data_read_discrete_inputs_byte(uint16_t addr, uint16_t remaining);

/**
 * @brief 检查离散输入地址范围
 */
bool modbus_data_check_discrete_input_range(uint16_t start_addr, uint16_t count);

/* ============================================================================
 * 保持寄存器操作
 * ============================================================================ */

/**
 * @brief 读保持寄存器
 * @param addr 寄存器地址
 * @return 寄存器值
 */
uint16_t modbus_data_read_holding_register(uint16_t addr);

/**
 * @brief 写保持寄存器
 * @param addr 寄存器地址
 * @param value 寄存器值
 */
void modbus_data_write_holding_register(uint16_t addr, uint16_t value);

/**
 * @brief 检查保持寄存器地址范围
 */
bool modbus_data_check_holding_register_range(uint16_t start_addr, uint16_t count);

/* ============================================================================
 * 输入寄存器操作
 * ============================================================================ */

/**
 * @brief 读输入寄存器
 * @param addr 寄存器地址
 * @return 寄存器值
 */
uint16_t modbus_data_read_input_register(uint16_t addr);

/**
 * @brief 检查输入寄存器地址范围
 */
bool modbus_data_check_input_register_range(uint16_t start_addr, uint16_t count);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_DATA_H */
