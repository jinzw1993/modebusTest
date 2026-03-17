/**
 * @file modbus_data.c
 * @brief Modbus数据区管理实现
 * @author Claude
 * @date 2026-03-17
 */

#include "modbus_data.h"
#include "modbus_config.h"
#include <string.h>

/* 使用配置文件中的定义 */
#ifndef MB_COILS_SIZE
#define MB_COILS_SIZE               1024
#endif

#ifndef MB_DISCRETE_INPUTS_SIZE
#define MB_DISCRETE_INPUTS_SIZE     1024
#endif

#ifndef MB_HOLDING_REGISTERS_SIZE
#define MB_HOLDING_REGISTERS_SIZE   512
#endif

#ifndef MB_INPUT_REGISTERS_SIZE
#define MB_INPUT_REGISTERS_SIZE     512
#endif

/* ============================================================================
 * 静态数据区（可选使用）
 * ============================================================================ */

/* 静态线圈区 */
static uint8_t s_coils[(MB_COILS_SIZE + 7) / 8];

/* 静态离散输入区 */
static uint8_t s_discrete_inputs[(MB_DISCRETE_INPUTS_SIZE + 7) / 8];

/* 静态保持寄存器区 */
static uint16_t s_holding_registers[MB_HOLDING_REGISTERS_SIZE];

/* 静态输入寄存器区 */
static uint16_t s_input_registers[MB_INPUT_REGISTERS_SIZE];

/* ============================================================================
 * 数据区实例
 * ============================================================================ */

static struct {
    /* 线圈区 */
    uint8_t *coils;
    uint16_t coils_size;
    mb_read_bit_cb_t read_coil_cb;
    mb_write_bit_cb_t write_coil_cb;

    /* 离散输入区 */
    uint8_t *discrete_inputs;
    uint16_t discrete_inputs_size;
    mb_read_bit_cb_t read_discrete_input_cb;

    /* 保持寄存器区 */
    uint16_t *holding_registers;
    uint16_t holding_registers_size;
    mb_read_reg_cb_t read_holding_reg_cb;
    mb_write_reg_cb_t write_holding_reg_cb;

    /* 输入寄存器区 */
    uint16_t *input_registers;
    uint16_t input_registers_size;
    mb_read_reg_cb_t read_input_reg_cb;

    /* 初始化标志 */
    uint8_t initialized;
} s_data = {0};

/* ============================================================================
 * 初始化
 * ============================================================================ */

int modbus_data_init(const mb_data_config_t *config)
{
    if (config == NULL) {
        return -1;
    }

    /* 线圈区 */
    s_data.coils = config->coils ? config->coils : s_coils;
    s_data.coils_size = config->coils_size > 0 ? config->coils_size : MB_COILS_SIZE;
    s_data.read_coil_cb = config->read_coil_cb;
    s_data.write_coil_cb = config->write_coil_cb;

    /* 离散输入区 */
    s_data.discrete_inputs = config->discrete_inputs ? config->discrete_inputs : s_discrete_inputs;
    s_data.discrete_inputs_size = config->discrete_inputs_size > 0 ?
                                   config->discrete_inputs_size : MB_DISCRETE_INPUTS_SIZE;
    s_data.read_discrete_input_cb = config->read_discrete_input_cb;

    /* 保持寄存器区 */
    s_data.holding_registers = config->holding_registers ?
                               config->holding_registers : s_holding_registers;
    s_data.holding_registers_size = config->holding_registers_size > 0 ?
                                    config->holding_registers_size : MB_HOLDING_REGISTERS_SIZE;
    s_data.read_holding_reg_cb = config->read_holding_reg_cb;
    s_data.write_holding_reg_cb = config->write_holding_reg_cb;

    /* 输入寄存器区 */
    s_data.input_registers = config->input_registers ?
                             config->input_registers : s_input_registers;
    s_data.input_registers_size = config->input_registers_size > 0 ?
                                  config->input_registers_size : MB_INPUT_REGISTERS_SIZE;
    s_data.read_input_reg_cb = config->read_input_reg_cb;

    /* 初始化静态数据区为0 */
    memset(s_coils, 0, sizeof(s_coils));
    memset(s_discrete_inputs, 0, sizeof(s_discrete_inputs));
    memset(s_holding_registers, 0, sizeof(s_holding_registers));
    memset(s_input_registers, 0, sizeof(s_input_registers));

    s_data.initialized = 1;

    return 0;
}

void modbus_data_deinit(void)
{
    s_data.initialized = 0;
}

/* ============================================================================
 * 线圈操作
 * ============================================================================ */

uint8_t modbus_data_read_coil(uint16_t addr)
{
    /* 优先使用回调 */
    if (s_data.read_coil_cb) {
        return s_data.read_coil_cb(addr);
    }

    /* 检查地址范围 */
    if (addr >= s_data.coils_size) {
        return 0;
    }

    /* 从缓冲区读取 */
    uint8_t byte_idx = addr / 8;
    uint8_t bit_idx = addr % 8;

    return (s_data.coils[byte_idx] >> bit_idx) & 0x01;
}

void modbus_data_write_coil(uint16_t addr, uint8_t value)
{
    /* 检查地址范围 */
    if (addr >= s_data.coils_size) {
        return;
    }

    /* 写入缓冲区 */
    uint8_t byte_idx = addr / 8;
    uint8_t bit_idx = addr % 8;

    if (value) {
        s_data.coils[byte_idx] |= (1 << bit_idx);
    } else {
        s_data.coils[byte_idx] &= ~(1 << bit_idx);
    }

    /* 调用回调通知应用层 */
    if (s_data.write_coil_cb) {
        s_data.write_coil_cb(addr, value);
    }
}

uint8_t modbus_data_read_coils_byte(uint16_t addr, uint16_t remaining)
{
    uint8_t result = 0;
    uint8_t i;
    uint8_t count = (remaining >= 8) ? 8 : (uint8_t)remaining;

    for (i = 0; i < count; i++) {
        if (modbus_data_read_coil(addr + i)) {
            result |= (1 << i);
        }
    }

    return result;
}

bool modbus_data_check_coil_range(uint16_t start_addr, uint16_t count)
{
    if (count == 0) {
        return false;
    }

    if (start_addr >= s_data.coils_size) {
        return false;
    }

    if ((uint32_t)start_addr + count > s_data.coils_size) {
        return false;
    }

    return true;
}

/* ============================================================================
 * 离散输入操作
 * ============================================================================ */

uint8_t modbus_data_read_discrete_input(uint16_t addr)
{
    /* 优先使用回调 */
    if (s_data.read_discrete_input_cb) {
        return s_data.read_discrete_input_cb(addr);
    }

    /* 检查地址范围 */
    if (addr >= s_data.discrete_inputs_size) {
        return 0;
    }

    /* 从缓冲区读取 */
    uint8_t byte_idx = addr / 8;
    uint8_t bit_idx = addr % 8;

    return (s_data.discrete_inputs[byte_idx] >> bit_idx) & 0x01;
}

uint8_t modbus_data_read_discrete_inputs_byte(uint16_t addr, uint16_t remaining)
{
    uint8_t result = 0;
    uint8_t i;
    uint8_t count = (remaining >= 8) ? 8 : (uint8_t)remaining;

    for (i = 0; i < count; i++) {
        if (modbus_data_read_discrete_input(addr + i)) {
            result |= (1 << i);
        }
    }

    return result;
}

bool modbus_data_check_discrete_input_range(uint16_t start_addr, uint16_t count)
{
    if (count == 0) {
        return false;
    }

    if (start_addr >= s_data.discrete_inputs_size) {
        return false;
    }

    if ((uint32_t)start_addr + count > s_data.discrete_inputs_size) {
        return false;
    }

    return true;
}

/* ============================================================================
 * 保持寄存器操作
 * ============================================================================ */

uint16_t modbus_data_read_holding_register(uint16_t addr)
{
    /* 优先使用回调 */
    if (s_data.read_holding_reg_cb) {
        return s_data.read_holding_reg_cb(addr);
    }

    /* 检查地址范围 */
    if (addr >= s_data.holding_registers_size) {
        return 0;
    }

    return s_data.holding_registers[addr];
}

void modbus_data_write_holding_register(uint16_t addr, uint16_t value)
{
    /* 检查地址范围 */
    if (addr >= s_data.holding_registers_size) {
        return;
    }

    /* 写入缓冲区 */
    s_data.holding_registers[addr] = value;

    /* 调用回调通知应用层 */
    if (s_data.write_holding_reg_cb) {
        s_data.write_holding_reg_cb(addr, value);
    }
}

bool modbus_data_check_holding_register_range(uint16_t start_addr, uint16_t count)
{
    if (count == 0) {
        return false;
    }

    if (start_addr >= s_data.holding_registers_size) {
        return false;
    }

    if ((uint32_t)start_addr + count > s_data.holding_registers_size) {
        return false;
    }

    return true;
}

/* ============================================================================
 * 输入寄存器操作
 * ============================================================================ */

uint16_t modbus_data_read_input_register(uint16_t addr)
{
    /* 优先使用回调 */
    if (s_data.read_input_reg_cb) {
        return s_data.read_input_reg_cb(addr);
    }

    /* 检查地址范围 */
    if (addr >= s_data.input_registers_size) {
        return 0;
    }

    return s_data.input_registers[addr];
}

bool modbus_data_check_input_register_range(uint16_t start_addr, uint16_t count)
{
    if (count == 0) {
        return false;
    }

    if (start_addr >= s_data.input_registers_size) {
        return false;
    }

    if ((uint32_t)start_addr + count > s_data.input_registers_size) {
        return false;
    }

    return true;
}
