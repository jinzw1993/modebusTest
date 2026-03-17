/**
 * @file modbus_types.h
 * @brief Modbus公共类型定义
 * @author Claude
 * @date 2026-03-17
 */

#ifndef MODBUS_TYPES_H
#define MODBUS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Modbus功能码定义
 * ============================================================================ */
typedef enum {
    MB_FC_READ_COILS                  = 0x01,  /* 读线圈 */
    MB_FC_READ_DISCRETE_INPUTS        = 0x02,  /* 读离散输入 */
    MB_FC_READ_HOLDING_REGISTERS      = 0x03,  /* 读保持寄存器 */
    MB_FC_READ_INPUT_REGISTERS        = 0x04,  /* 读输入寄存器 */
    MB_FC_WRITE_SINGLE_COIL           = 0x05,  /* 写单个线圈 */
    MB_FC_WRITE_SINGLE_REGISTER       = 0x06,  /* 写单个寄存器 */
    MB_FC_READ_EXCEPTION_STATUS       = 0x07,  /* 读异常状态（可选） */
    MB_FC_DIAGNOSTICS                 = 0x08,  /* 诊断（可选） */
    MB_FC_WRITE_MULTIPLE_COILS        = 0x0F,  /* 写多个线圈 */
    MB_FC_WRITE_MULTIPLE_REGISTERS    = 0x10,  /* 写多个寄存器 */
    MB_FC_REPORT_SLAVE_ID             = 0x11,  /* 报告从站ID（可选） */
} mb_function_code_t;

/* ============================================================================
 * Modbus异常码定义
 * ============================================================================ */
typedef enum {
    MB_EX_NONE                        = 0x00,  /* 无异常 */
    MB_EX_ILLEGAL_FUNCTION            = 0x01,  /* 非法功能码 */
    MB_EX_ILLEGAL_DATA_ADDRESS        = 0x02,  /* 非法数据地址 */
    MB_EX_ILLEGAL_DATA_VALUE          = 0x03,  /* 非法数据值 */
    MB_EX_SLAVE_DEVICE_FAILURE        = 0x04,  /* 从站设备故障 */
    MB_EX_ACKNOWLEDGE                 = 0x05,  /* 确认（长时间操作） */
    MB_EX_SLAVE_DEVICE_BUSY           = 0x06,  /* 从站设备忙 */
    MB_EX_MEMORY_PARITY_ERROR         = 0x08,  /* 内存奇偶校验错误 */
    MB_EX_GATEWAY_PATH_UNAVAILABLE    = 0x0A,  /* 网关路径不可用 */
    MB_EX_GATEWAY_TARGET_NO_RESPONSE  = 0x0B,  /* 网关目标无响应 */
} mb_exception_t;

/* ============================================================================
 * 内部错误码定义
 * ============================================================================ */
typedef enum {
    MB_ERROR_NONE                     = 0,     /* 无错误 */
    MB_ERROR_CRC                      = 1,     /* CRC校验错误 */
    MB_ERROR_FRAME_SIZE               = 2,     /* 帧长度错误 */
    MB_ERROR_ILLEGAL_ADDRESS          = 3,     /* 非法从站地址 */
    MB_ERROR_BUFFER_OVERFLOW          = 4,     /* 缓冲区溢出 */
    MB_ERROR_ILLEGAL_FUNCTION         = 5,     /* 非法功能码 */
    MB_ERROR_ILLEGAL_DATA_ADDR        = 6,     /* 非法数据地址 */
    MB_ERROR_ILLEGAL_DATA_VALUE       = 7,     /* 非法数据值 */
    MB_ERROR_SLAVE_BUSY               = 8,     /* 从站忙 */
    MB_ERROR_UNKNOWN                  = 9,     /* 未知错误 */
} mb_error_t;

/* ============================================================================
 * 状态机状态定义
 * ============================================================================ */
typedef enum {
    MB_STATE_IDLE                     = 0,     /* 空闲状态 */
    MB_STATE_RECEIVING                = 1,     /* 接收中 */
    MB_STATE_PROCESSING               = 2,     /* 处理中 */
    MB_STATE_RESPONDING               = 3,     /* 响应中 */
    MB_STATE_ERROR                    = 4,     /* 错误状态 */
} mb_state_t;

/* ============================================================================
 * 状态机事件定义
 * ============================================================================ */
typedef enum {
    MB_EVENT_NONE                     = 0,     /* 无事件 */
    MB_EVENT_BYTE_RECEIVED            = 1,     /* 接收到字节 */
    MB_EVENT_T35_EXPIRED              = 2,     /* T3.5超时 */
    MB_EVENT_PROCESS_COMPLETE         = 3,     /* 处理完成 */
    MB_EVENT_TX_COMPLETE              = 4,     /* 发送完成 */
    MB_EVENT_ERROR_DETECTED           = 5,     /* 检测到错误 */
    MB_EVENT_RESET                    = 6,     /* 复位事件 */
} mb_event_t;

/* ============================================================================
 * 数据区类型定义
 * ============================================================================ */
typedef enum {
    MB_AREA_COILS                     = 0,     /* 线圈区 */
    MB_AREA_DISCRETE_INPUTS           = 1,     /* 离散输入区 */
    MB_AREA_HOLDING_REGISTERS         = 2,     /* 保持寄存器区 */
    MB_AREA_INPUT_REGISTERS           = 3,     /* 输入寄存器区 */
} mb_data_area_type_t;

/* ============================================================================
 * 回调函数类型定义
 * ============================================================================ */

/**
 * @brief 读寄存器回调函数类型
 * @param addr 寄存器地址
 * @return 寄存器值
 */
typedef uint16_t (*mb_read_reg_cb_t)(uint16_t addr);

/**
 * @brief 写寄存器回调函数类型
 * @param addr 寄存器地址
 * @param value 寄存器值
 */
typedef void (*mb_write_reg_cb_t)(uint16_t addr, uint16_t value);

/**
 * @brief 读位（线圈/离散输入）回调函数类型
 * @param addr 位地址
 * @return 位值 (0或1)
 */
typedef uint8_t (*mb_read_bit_cb_t)(uint16_t addr);

/**
 * @brief 写位（线圈）回调函数类型
 * @param addr 位地址
 * @param value 位值 (0或1)
 */
typedef void (*mb_write_bit_cb_t)(uint16_t addr, uint8_t value);

/**
 * @brief 错误回调函数类型
 * @param error 错误码
 */
typedef void (*mb_error_cb_t)(mb_error_t error);

/**
 * @brief 空回调函数类型
 */
typedef void (*mb_callback_t)(void);

/* ============================================================================
 * 常量定义
 * ============================================================================ */

/* 从站地址范围 */
#define MB_SLAVE_ADDR_MIN           1       /* 最小从站地址 */
#define MB_SLAVE_ADDR_MAX           247     /* 最大从站地址 */
#define MB_SLAVE_ADDR_BROADCAST     0       /* 广播地址 */

/* 帧长度限制 */
#define MB_RTU_FRAME_MIN_SIZE       4       /* 最小帧长度：地址+功能码+CRC16 */
#define MB_RTU_FRAME_MAX_SIZE       256     /* 最大帧长度 */

/* 功能码相关限制 */
#define MB_MAX_REGISTERS_PER_REQ    125     /* 单次请求最大寄存器数 */
#define MB_MAX_COILS_PER_REQ        2000    /* 单次请求最大线圈数 */

/* 异常响应标志 */
#define MB_EXCEPTION_BIT            0x80    /* 异常响应功能码最高位 */

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_TYPES_H */
