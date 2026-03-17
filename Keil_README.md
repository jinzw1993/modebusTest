# Keil项目使用说明

## 项目文件

- `modbusTest.uvprojx` - Keil uVision 5 项目文件

## 使用步骤

### 1. 安装必要软件

1. 安装 Keil MDK-ARM (uVision 5)
2. 安装 AT32F423 设备支持包：
   - 打开 Keil
   - 菜单：Pack Installer
   - 搜索并安装 `ArteryTek.AT32F423_DFP`

### 2. 下载AT32标准外设库

从雅特力官网或GitHub下载 AT32F423 标准外设库，并放置到 `Libraries` 目录：

GitHub: https://github.com/ArteryTek/AT32F423_Firmware_Library

```
modebusTest/
├── Libraries/
│   ├── drivers/
│   │   ├── inc/
│   │   │   ├── at32f423.h
│   │   │   ├── at32f423_gpio.h
│   │   │   ├── at32f423_usart.h
│   │   │   ├── at32f423_tmr.h
│   │   │   ├── at32f423_crm.h
│   │   │   └── ...
│   │   └── src/
│   │       ├── at32f423_gpio.c
│   │       ├── at32f423_usart.c
│   │       ├── at32f423_tmr.c
│   │       ├── at32f423_crm.c
│   │       └── ...
│   └── cmsis/
│       └── cm4/
│           ├── core_support/
│           │   ├── core_cm4.h
│           │   └── ...
│           └── device_support/
│               ├── at32f423.h
│               ├── system_at32f423.c
│               └── startup/
│                   └── startup_at32f423.s
├── src/
├── inc/
└── modbusTest.uvprojx
```

### 3. 打开项目

双击 `modbusTest.uvprojx` 打开项目

### 4. 配置项目

#### 4.1 选择目标芯片（如需要）

- Project -> Options for Target -> Device
- 选择正确的 AT32F423 型号（如 AT32F423K8U7_4）

#### 4.2 配置Include路径

- Project -> Options for Target -> C/C++
- 确保Include Paths包含：
  ```
  inc;src;src\protocol;src\hal;src\app;src\port;Libraries\drivers\inc;Libraries\cmsis\cm4\core_support;Libraries\cmsis\cm4\device_support
  ```

#### 4.3 配置预处理器宏

- Project -> Options for Target -> C/C++
- Define中添加：
  ```
  USE_STDPERIPH_DRIVER, AT32F423K8U7_4
  ```

### 5. 编译项目

- 点击 Build (F7) 编译项目

## 硬件配置

### 默认配置

| 功能 | 引脚 | 说明 |
|------|------|------|
| USART1_TX | PA9 | Modbus发送 |
| USART1_RX | PA10 | Modbus接收 |
| TMR3 | - | T3.5定时器 |

### 修改串口配置

如需使用其他串口，修改 `src/port/port_uart_at32.c` 中的宏定义：

```c
#define PORT_USART              USART1
#define PORT_USART_TX_GPIO      GPIOA
#define PORT_USART_TX_PIN       GPIO_PINS_9
#define PORT_USART_RX_GPIO      GPIOA
#define PORT_USART_RX_PIN       GPIO_PINS_10
```

### 修改定时器配置

如需使用其他定时器，修改 `src/port/port_timer_at32.c` 中的宏定义：

```c
#define PORT_TMR                TMR3
#define PORT_TMR_IRQn           TMR3_GLOBAL_IRQn
```

## Modbus配置

在 `src/app/modbus_config.h` 中可以修改：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| MB_SLAVE_ID_DEFAULT | 1 | 从站地址 |
| MB_UART_BAUDRATE_DEFAULT | 9600 | 波特率 |
| MB_UART_PARITY_DEFAULT | 2 | 校验（0=无, 1=奇, 2=偶）|
| MB_T35_TIMEOUT_MS | 10 | T3.5超时(ms) |

## 常见问题

### 1. 找不到头文件

- 检查Include路径是否正确
- 确认AT32标准外设库已正确放置

### 2. 链接错误

- 检查是否添加了所有必要的源文件
- 确认启动文件是否正确

### 3. 下载失败

- 检查调试器配置
- 确认芯片型号匹配

## 项目结构

```
src/
├── main.c              # 主程序
├── protocol/           # Modbus协议层
│   ├── modbus_rtu.c   # RTU帧处理
│   ├── modbus_pdu.c   # PDU处理
│   └── modbus_crc.c   # CRC校验
├── hal/               # 硬件抽象层
│   ├── hal_uart.c     # 串口HAL
│   └── hal_timer.c    # 定时器HAL
├── app/               # 应用层
│   ├── modbus_slave.c # 从站接口
│   └── modbus_data.c  # 数据区管理
└── port/              # 移植层
    ├── port_uart_at32.c  # AT32串口移植
    └── port_timer_at32.c # AT32定时器移植
```
