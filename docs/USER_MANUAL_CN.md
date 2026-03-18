# Modbus RTU从站 使用手册

## 概述

本项目实现了一个完整的Modbus RTU从站协议栈，专为AT32F423系列微控制器设计。采用硬件抽象层(HAL)架构，便于移植到其他平台。

## 功能特性

- 支持Modbus RTU帧格式
- 支持标准Modbus功能码：
  - 0x01: 读线圈
  - 0x02: 读离散输入
  - 0x03: 读保持寄存器
  - 0x04: 读输入寄存器
  - 0x05: 写单个线圈
  - 0x06: 写单个寄存器
  - 0x0F: 写多个线圈
  - 0x10: 写多个寄存器
- 支持广播地址(0x00)
- 可配置的数据区与回调函数
- 硬件抽象层设计，易于移植
- 可选的统计功能
- 可选的调试输出

## 快速入门

### 1. 硬件要求

- AT32F423系列MCU
- USART1 (PA9=TX, PA10=RX) 用于Modbus通信
- TMR3 用于T3.5超时检测
- RS485收发器（可选，用于长距离通信）

### 2. 项目配置

#### 2.1 包含路径

在Keil项目中添加以下包含路径：
```
inc
src
src\protocol
src\hal
src\app
src\port
Libraries\drivers\inc
Libraries\cmsis\cm4\core_support
Libraries\cmsis\cm4\device_support
```

#### 2.2 预处理器宏

添加以下宏定义：
```
USE_STDPERIPH_DRIVER
AT32F423K8U7_4
```

### 3. 配置选项

编辑 `src/app/modbus_config.h` 进行配置：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| MB_SLAVE_ID_DEFAULT | 1 | 默认从站地址(1-247) |
| MB_UART_BAUDRATE_DEFAULT | 9600 | 默认波特率 |
| MB_UART_PARITY_DEFAULT | 2 | 校验: 0=无, 1=奇, 2=偶 |
| MB_T35_TIMEOUT_MS | 10 | T3.5超时时间(ms) |
| MB_BUFFER_SIZE | 256 | 收发缓冲区大小 |
| MB_SUPPORT_BROADCAST | 1 | 启用广播支持 |
| MB_ENABLE_STATS | 1 | 启用统计功能 |
| MB_DEBUG_ENABLE | 0 | 启用调试输出 |

### 4. 基本使用

#### 4.1 初始化

```c
#include "modbus_slave.h"
#include "modbus_data.h"
#include "modbus_config.h"
#include "hal_uart.h"
#include "hal_timer.h"

/* 数据缓冲区 */
static uint8_t coils_buffer[128];
static uint8_t discrete_inputs_buffer[128];
static uint16_t holding_registers_buffer[256];
static uint16_t input_registers_buffer[256];

/* 回调函数示例 */
static void on_holding_reg_written(uint16_t addr, uint16_t value)
{
    /* 处理寄存器写入 */
    if (addr == 0) {
        /* 控制寄存器 */
        if (value & 0x0001) {
            start_motor();  /* 启动电机 */
        }
    }
}

static uint16_t on_input_reg_read(uint16_t addr)
{
    /* 返回传感器数据 */
    switch (addr) {
        case 0: return read_temperature();  /* 温度 */
        case 1: return read_humidity();     /* 湿度 */
        default: return input_registers_buffer[addr];
    }
}

void modbus_init(void)
{
    /* 注册HAL驱动 */
    port_uart_at32_register();
    port_timer_at32_register();

    /* 配置数据区 */
    mb_data_config_t data_cfg = {
        /* 线圈 */
        .coils = coils_buffer,
        .coils_size = 1024,
        .write_coil_cb = NULL,  /* 直接使用缓冲区 */

        /* 离散输入 */
        .discrete_inputs = discrete_inputs_buffer,
        .discrete_inputs_size = 1024,
        .read_discrete_input_cb = NULL,

        /* 保持寄存器 */
        .holding_registers = holding_registers_buffer,
        .holding_registers_size = 256,
        .write_holding_reg_cb = on_holding_reg_written,

        /* 输入寄存器 */
        .input_registers = input_registers_buffer,
        .input_registers_size = 256,
        .read_input_reg_cb = on_input_reg_read
    };

    modbus_data_init(&data_cfg);

    /* 初始化从站 */
    modbus_slave_init(MB_SLAVE_ID_DEFAULT);

    /* 初始化硬件 */
    hal_uart_init(MB_UART_BAUDRATE_DEFAULT, HAL_UART_PARITY_EVEN);
    hal_timer_init(MB_T35_TIMEOUT_MS);
}
```

#### 4.2 主循环

```c
int main(void)
{
    /* 系统初始化 */
    system_clock_config();
    modbus_init();

    /* 主循环 */
    while (1) {
        /* Modbus轮询 */
        modbus_slave_poll();

        /* 应用层任务 */
        update_sensors();
        process_control();
    }

    return 0;
}
```

### 5. 数据区配置

#### 5.1 缓冲区模式（简单）

最简单的方式是使用静态缓冲区，协议栈直接读写缓冲区。

```c
mb_data_config_t data_cfg = {
    .coils = coils_buffer,
    .coils_size = 1024,
    .read_coil_cb = NULL,
    .write_coil_cb = NULL,
    // ... 其他字段
};
```

#### 5.2 回调模式（灵活）

使用回调函数适用于：
- 从硬件读取数据（传感器、ADC）
- 控制硬件输出（继电器、DAC）
- 实现虚拟寄存器
- 添加数据验证逻辑

```c
/* 读回调 - 返回数据 */
static uint16_t on_input_reg_read(uint16_t addr)
{
    return read_adc_value(addr);
}

/* 写回调 - 执行动作 */
static void on_coil_written(uint16_t addr, uint8_t value)
{
    set_relay(addr, value);
}

mb_data_config_t data_cfg = {
    .input_registers = NULL,  /* 不使用缓冲区 */
    .input_registers_size = 0,
    .read_input_reg_cb = on_input_reg_read,

    .coils = coils_buffer,
    .coils_size = 16,
    .write_coil_cb = on_coil_written,
    // ... 其他字段
};
```

### 6. 寄存器映射示例

| 地址 | 类型 | 说明 | 访问 |
|------|------|------|------|
| 0 | 线圈 | 继电器1 | 读写 |
| 1 | 线圈 | 继电器2 | 读写 |
| 0 | 离散输入 | 限位开关1 | 只读 |
| 1 | 离散输入 | 限位开关2 | 只读 |
| 0 | 保持寄存器 | 控制字 | 读写 |
| 1 | 保持寄存器 | 设定值 | 读写 |
| 0 | 输入寄存器 | 温度(0.1°C) | 只读 |
| 1 | 输入寄存器 | 湿度(0.1%) | 只读 |

### 7. 使用Modbus Poll测试

可以使用Modbus Poll软件进行测试：

1. 配置连接：9600, 8N1（8数据位，无校验，1停止位）
2. 设置从站地址：1
3. 读线圈：功能码01，地址0，数量10
4. 读寄存器：功能码03，地址0，数量10

### 8. 常见问题

#### 无响应
- 检查RS485接线（A+、B-）
- 验证波特率和校验设置
- 检查从站地址是否匹配

#### CRC错误
- 检查电气干扰
- 验证波特率是否与主机匹配
- 检查电缆长度和终端电阻

#### 超时
- 增大T3.5超时值
- 检查发送是否有帧间隙

### 9. 性能说明

- 中断驱动接收，确保及时捕获每个字节
- T3.5定时器自动检测帧边界
- 帧处理完成后立即发送响应
- 典型响应时间：< 10ms @ 9600波特率

### 10. 内存占用

| 组件 | RAM | Flash |
|------|-----|-------|
| 协议栈 | 约1.3KB | 约8KB |
| 数据缓冲区 | 可配置 | - |
| HAL层 | 约100B | 约2KB |

---

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.1 | 2026-03-18 | 更新HAL层API文档 |
| 1.0 | 2026-03-17 | 初始版本 |
