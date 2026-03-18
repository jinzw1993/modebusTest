# Modbus RTU从站 API参考手册

## 目录

1. [Modbus从站API](#1-modbus从站api)
2. [Modbus数据API](#2-modbus数据api)
3. [HAL串口API](#3-hal串口api)
4. [HAL定时器API](#4-hal定时器api)
5. [数据结构](#5-数据结构)
6. [错误码](#6-错误码)
7. [异常码](#7-异常码)

---

## 1. Modbus从站API

### modbus_slave_init

**功能：** 初始化Modbus从站

**原型：**
```c
int modbus_slave_init(uint8_t slave_id);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| slave_id | uint8_t | 从站地址(1-247) |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 无效的从站地址 |

**示例：**
```c
modbus_slave_init(1);  // 设置从站地址为1
```

---

### modbus_slave_deinit

**功能：** 反初始化Modbus从站

**原型：**
```c
void modbus_slave_deinit(void);
```

**示例：**
```c
modbus_slave_deinit();
```

---

### modbus_slave_poll

**功能：** 状态机轮询函数（在主循环中调用）

**原型：**
```c
void modbus_slave_poll(void);
```

**示例：**
```c
while (1) {
    modbus_slave_poll();
}
```

---

### modbus_slave_set_address

**功能：** 运行时修改从站地址

**原型：**
```c
void modbus_slave_set_address(uint8_t slave_id);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| slave_id | uint8_t | 新的从站地址(1-247) |

**示例：**
```c
modbus_slave_set_address(10);
```

---

### modbus_slave_get_address

**功能：** 获取当前从站地址

**原型：**
```c
uint8_t modbus_slave_get_address(void);
```

**返回值：** 当前从站地址

**示例：**
```c
uint8_t addr = modbus_slave_get_address();
```

---

### modbus_slave_get_state

**功能：** 获取当前状态机状态

**原型：**
```c
mb_state_t modbus_slave_get_state(void);
```

**返回值：**
| 状态 | 说明 |
|------|------|
| MB_STATE_IDLE | 空闲，等待请求 |
| MB_STATE_RECEIVING | 正在接收帧 |
| MB_STATE_PROCESSING | 正在处理请求 |
| MB_STATE_RESPONDING | 正在发送响应 |
| MB_STATE_ERROR | 错误状态 |

---

### modbus_slave_set_rx_callback

**功能：** 设置帧接收完成回调

**原型：**
```c
void modbus_slave_set_rx_callback(mb_callback_t callback);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| callback | mb_callback_t | 回调函数 (void (*)(void)) |

**示例：**
```c
void on_frame_received(void) {
    // 帧已接收
}

modbus_slave_set_rx_callback(on_frame_received);
```

---

### modbus_slave_set_tx_callback

**功能：** 设置发送完成回调

**原型：**
```c
void modbus_slave_set_tx_callback(mb_callback_t callback);
```

**示例：**
```c
void on_frame_sent(void) {
    // 响应已发送
}

modbus_slave_set_tx_callback(on_frame_sent);
```

---

### modbus_slave_set_error_callback

**功能：** 设置错误回调

**原型：**
```c
void modbus_slave_set_error_callback(mb_error_cb_t callback);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| callback | mb_error_cb_t | 回调函数 (void (*)(uint8_t error)) |

**示例：**
```c
void on_error(uint8_t error) {
    printf("Error: %d\n", error);
}

modbus_slave_set_error_callback(on_error);
```

---

### modbus_slave_get_stats

**功能：** 获取通信统计信息

**原型：**
```c
void modbus_slave_get_stats(mb_stats_t *stats);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| stats | mb_stats_t* | 统计结构体指针 |

**示例：**
```c
mb_stats_t stats;
modbus_slave_get_stats(&stats);
printf("RX: %lu, TX: %lu, Errors: %lu\n",
       stats.rx_count, stats.tx_count, stats.error_count);
```

---

### modbus_slave_reset_stats

**功能：** 重置统计计数器

**原型：**
```c
void modbus_slave_reset_stats(void);
```

---

## 2. Modbus数据API

### modbus_data_init

**功能：** 初始化数据区配置

**原型：**
```c
int modbus_data_init(const mb_data_config_t *config);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| config | mb_data_config_t* | 数据配置结构体 |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 参数无效 |

**示例：**
```c
mb_data_config_t data_cfg = {
    .coils = coils_buffer,
    .coils_size = 1024,
    .read_coil_cb = NULL,
    .write_coil_cb = on_coil_written,
    // ... 其他字段
};
modbus_data_init(&data_cfg);
```

---

### modbus_data_read_coil

**功能：** 读取线圈值

**原型：**
```c
int modbus_data_read_coil(uint16_t addr, uint8_t *value);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| addr | uint16_t | 线圈地址 |
| value | uint8_t* | 输出值(0或1) |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 地址越界 |

---

### modbus_data_write_coil

**功能：** 写入线圈值

**原型：**
```c
int modbus_data_write_coil(uint16_t addr, uint8_t value);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| addr | uint16_t | 线圈地址 |
| value | uint8_t | 写入值(0或1) |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 地址越界 |

---

### modbus_data_read_discrete_input

**功能：** 读取离散输入值

**原型：**
```c
int modbus_data_read_discrete_input(uint16_t addr, uint8_t *value);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| addr | uint16_t | 离散输入地址 |
| value | uint8_t* | 输出值(0或1) |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 地址越界 |

---

### modbus_data_read_holding_reg

**功能：** 读取保持寄存器值

**原型：**
```c
int modbus_data_read_holding_reg(uint16_t addr, uint16_t *value);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| addr | uint16_t | 寄存器地址 |
| value | uint16_t* | 输出值 |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 地址越界 |

---

### modbus_data_write_holding_reg

**功能：** 写入保持寄存器值

**原型：**
```c
int modbus_data_write_holding_reg(uint16_t addr, uint16_t value);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| addr | uint16_t | 寄存器地址 |
| value | uint16_t | 写入值 |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 地址越界 |

---

### modbus_data_read_input_reg

**功能：** 读取输入寄存器值

**原型：**
```c
int modbus_data_read_input_reg(uint16_t addr, uint16_t *value);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| addr | uint16_t | 寄存器地址 |
| value | uint16_t* | 输出值 |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 地址越界 |

---

## 3. HAL串口API

### hal_uart_init

**功能：** 初始化串口

**原型：**
```c
int hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| baudrate | uint32_t | 波特率(如9600, 19200) |
| parity | hal_uart_parity_t | 校验设置 |

**校验值：**
| 值 | 说明 |
|----|------|
| HAL_UART_PARITY_NONE | 无校验 |
| HAL_UART_PARITY_ODD | 奇校验 |
| HAL_UART_PARITY_EVEN | 偶校验 |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 错误 |

---

### hal_uart_deinit

**功能：** 反初始化串口

**原型：**
```c
void hal_uart_deinit(void);
```

---

### hal_uart_send

**功能：** 异步发送数据

**原型：**
```c
int hal_uart_send(const uint8_t *data, uint16_t len);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| data | const uint8_t* | 数据缓冲区 |
| len | uint16_t | 数据长度 |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 忙或错误 |

---

### hal_uart_set_rx_callback

**功能：** 设置字节接收回调

**原型：**
```c
void hal_uart_set_rx_callback(void (*callback)(uint8_t byte));
```

---

### hal_uart_set_tx_complete_callback

**功能：** 设置发送完成回调

**原型：**
```c
void hal_uart_set_tx_complete_callback(void (*callback)(void));
```

---

### hal_uart_is_tx_busy

**功能：** 检查发送是否忙碌

**原型：**
```c
int hal_uart_is_tx_busy(void);
```

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 空闲 |
| 非零 | 忙碌 |

---

### hal_uart_get_rx_errors

**功能：** 获取接收错误计数

**原型：**
```c
uint32_t hal_uart_get_rx_errors(void);
```

**返回值：** 累计接收错误次数

---

### hal_uart_clear_rx_errors

**功能：** 清除接收错误计数

**原型：**
```c
void hal_uart_clear_rx_errors(void);
```

---

### hal_uart_register

**功能：** 注册HAL串口驱动

**原型：**
```c
int hal_uart_register(const hal_uart_t *uart);
```

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 参数无效 |

---

## 4. HAL定时器API

### hal_timer_init

**功能：** 初始化定时器

**原型：**
```c
int hal_timer_init(uint32_t timeout_ms);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| timeout_ms | uint32_t | 超时时间(毫秒) |

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 错误 |

---

### hal_timer_deinit

**功能：** 反初始化定时器

**原型：**
```c
void hal_timer_deinit(void);
```

---

### hal_timer_start

**功能：** 启动定时器

**原型：**
```c
void hal_timer_start(void);
```

---

### hal_timer_stop

**功能：** 停止定时器

**原型：**
```c
void hal_timer_stop(void);
```

---

### hal_timer_reset

**功能：** 重置定时器计数

**原型：**
```c
void hal_timer_reset(void);
```

---

### hal_timer_set_timeout

**功能：** 设置超时时间

**原型：**
```c
void hal_timer_set_timeout(uint32_t timeout_ms);
```

**参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| timeout_ms | uint32_t | 超时时间(毫秒) |

---

### hal_timer_set_callback

**功能：** 设置超时回调

**原型：**
```c
void hal_timer_set_callback(void (*callback)(void));
```

---

### hal_timer_is_expired

**功能：** 检查定时器是否超时(轮询模式)

**原型：**
```c
bool hal_timer_is_expired(void);
```

**返回值：**
| 值 | 说明 |
|----|------|
| true | 已超时 |
| false | 未超时 |

---

### hal_timer_is_running

**功能：** 检查定时器是否正在运行

**原型：**
```c
bool hal_timer_is_running(void);
```

**返回值：**
| 值 | 说明 |
|----|------|
| true | 正在运行 |
| false | 未运行 |

---

### hal_timer_register

**功能：** 注册HAL定时器驱动

**原型：**
```c
int hal_timer_register(const hal_timer_t *timer);
```

**返回值：**
| 值 | 说明 |
|----|------|
| 0 | 成功 |
| -1 | 参数无效 |

---

## 5. 数据结构

### mb_data_config_t

**说明：** 数据区配置结构体

```c
typedef struct {
    /* 线圈 */
    uint8_t *coils;              /* 线圈缓冲区 */
    uint16_t coils_size;         /* 线圈数量 */
    uint8_t (*read_coil_cb)(uint16_t addr);        /* 读回调 */
    void (*write_coil_cb)(uint16_t addr, uint8_t value); /* 写回调 */

    /* 离散输入 */
    uint8_t *discrete_inputs;    /* 离散输入缓冲区 */
    uint16_t discrete_inputs_size;
    uint8_t (*read_discrete_input_cb)(uint16_t addr);

    /* 保持寄存器 */
    uint16_t *holding_registers; /* 保持寄存器缓冲区 */
    uint16_t holding_registers_size;
    uint16_t (*read_holding_reg_cb)(uint16_t addr);
    void (*write_holding_reg_cb)(uint16_t addr, uint16_t value);

    /* 输入寄存器 */
    uint16_t *input_registers;   /* 输入寄存器缓冲区 */
    uint16_t input_registers_size;
    uint16_t (*read_input_reg_cb)(uint16_t addr);
} mb_data_config_t;
```

---

### mb_stats_t

**说明：** 通信统计结构体

```c
typedef struct {
    uint32_t rx_count;          /* 接收帧数 */
    uint32_t tx_count;          /* 发送帧数 */
    uint32_t error_count;       /* 总错误数 */
    uint32_t crc_error_count;   /* CRC错误数 */
    uint32_t broadcast_count;   /* 广播帧数 */
} mb_stats_t;
```

---

### mb_state_t

**说明：** 状态机状态枚举

```c
typedef enum {
    MB_STATE_IDLE = 0,      /* 空闲 */
    MB_STATE_RECEIVING,     /* 接收中 */
    MB_STATE_PROCESSING,    /* 处理中 */
    MB_STATE_RESPONDING,    /* 响应中 */
    MB_STATE_ERROR          /* 错误 */
} mb_state_t;
```

---

### hal_uart_t

**说明：** 串口HAL接口结构体

```c
typedef struct {
    int  (*init)(uint32_t baudrate, hal_uart_parity_t parity);
    void (*deinit)(void);
    int  (*send)(const uint8_t *data, uint16_t len);
    int  (*is_tx_busy)(void);
    uint32_t (*get_rx_errors)(void);
    void (*clear_rx_errors)(void);
    void (*set_rx_callback)(void (*callback)(uint8_t byte));
    void (*set_tx_complete_callback)(void (*callback)(void));
} hal_uart_t;
```

---

### hal_timer_t

**说明：** 定时器HAL接口结构体

```c
typedef struct hal_timer {
    int  (*init)(uint32_t timeout_ms);
    void (*deinit)(void);
    void (*start)(void);
    void (*stop)(void);
    void (*reset)(void);
    void (*set_timeout)(uint32_t timeout_ms);
    void (*set_callback)(void (*callback)(void));
    bool (*is_expired)(void);
    bool (*is_running)(void);
} hal_timer_t;
```

---

## 6. 错误码

| 码 | 名称 | 说明 |
|----|------|------|
| 0 | MB_ERROR_NONE | 无错误 |
| 1 | MB_ERROR_CRC | CRC校验错误 |
| 2 | MB_ERROR_FRAME | 帧格式错误 |
| 3 | MB_ERROR_BUFFER_OVERFLOW | 缓冲区溢出 |
| 4 | MB_ERROR_INVALID_FUNCTION | 无效功能码 |
| 5 | MB_ERROR_INVALID_ADDRESS | 无效地址 |
| 6 | MB_ERROR_INVALID_VALUE | 无效数值 |
| 7 | MB_ERROR_SLAVE_BUSY | 从站忙 |

---

## 7. 异常码

| 码 | 名称 | 说明 |
|----|------|------|
| 0x01 | MB_EX_ILLEGAL_FUNCTION | 非法功能码 |
| 0x02 | MB_EX_ILLEGAL_ADDRESS | 非法数据地址 |
| 0x03 | MB_EX_ILLEGAL_VALUE | 非法数据值 |
| 0x04 | MB_EX_SLAVE_FAILURE | 从站设备故障 |
| 0x05 | MB_EX_ACKNOWLEDGE | 确认（长时间操作） |
| 0x06 | MB_EX_SLAVE_BUSY | 从站设备忙 |
| 0x08 | MB_EX_MEMORY_PARITY | 内存奇偶校验错误 |

---

## 回调函数类型

### mb_callback_t

```c
typedef void (*mb_callback_t)(void);
```

### mb_error_cb_t

```c
typedef void (*mb_error_cb_t)(uint8_t error);
```

### hal_uart_parity_t

```c
typedef enum {
    HAL_UART_PARITY_NONE = 0,  /* 无校验 */
    HAL_UART_PARITY_ODD = 1,   /* 奇校验 */
    HAL_UART_PARITY_EVEN = 2   /* 偶校验 */
} hal_uart_parity_t;
```

---

## 移植指南

### 步骤1：创建移植文件

为您的平台创建 `port_uart_xxx.c` 和 `port_timer_xxx.c` 文件。

### 步骤2：实现HAL接口

实现所有必需的函数：

**串口：**
- `init` - 初始化串口（波特率、校验）
- `deinit` - 反初始化串口
- `send` - 异步发送数据
- `is_tx_busy` - 检查发送是否忙碌
- `get_rx_errors` - 获取接收错误计数
- `clear_rx_errors` - 清除接收错误计数
- `set_rx_callback` - 设置接收回调
- `set_tx_complete_callback` - 设置发送完成回调

**定时器：**
- `init` - 初始化定时器（超时时间）
- `deinit` - 反初始化定时器
- `start` - 启动定时器
- `stop` - 停止定时器
- `reset` - 重置计数器
- `set_timeout` - 设置超时时间
- `set_callback` - 设置超时回调
- `is_expired` - 检查是否超时
- `is_running` - 检查是否正在运行

### 步骤3：注册HAL驱动

```c
void port_uart_xxx_register(void) {
    hal_uart_register(&hal_uart_xxx);
}

void port_timer_xxx_register(void) {
    hal_timer_register(&hal_timer_xxx);
}
```

### 步骤4：在主程序中使用

```c
/* 声明注册函数 */
extern void port_uart_xxx_register(void);
extern void port_timer_xxx_register(void);

void modbus_init(void)
{
    /* 注册HAL驱动 */
    port_uart_xxx_register();
    port_timer_xxx_register();

    /* ... 其他初始化 */
}
```

---

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.1 | 2026-03-18 | 新增UART错误统计和定时器运行状态API |
| 1.0 | 2026-03-17 | 初始版本 |
