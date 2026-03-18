# Modbus RTU Slave User Manual

## Overview

This project implements a Modbus RTU slave protocol stack for AT32F423 microcontrollers. It provides a complete, portable Modbus RTU slave solution with hardware abstraction layer (HAL) design.

## Features

- Supports Modbus RTU frame format
- Supports standard Modbus function codes:
  - 0x01: Read Coils
  - 0x02: Read Discrete Inputs
  - 0x03: Read Holding Registers
  - 0x04: Read Input Registers
  - 0x05: Write Single Coil
  - 0x06: Write Single Register
  - 0x0F: Write Multiple Coils
  - 0x10: Write Multiple Registers
- Supports broadcast address (0x00)
- Configurable data areas with callbacks
- Hardware abstraction layer for easy porting
- Statistics support (optional)
- Debug output support (optional)

## Quick Start

### 1. Hardware Requirements

- AT32F423 series MCU
- USART1 (PA9=TX, PA10=RX) for Modbus communication
- TMR3 for T3.5 timeout detection
- RS485 transceiver (optional, for long-distance communication)

### 2. Project Configuration

#### 2.1 Include Paths

Add the following directories to your project include paths:
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

#### 2.2 Preprocessor Defines

Add the following defines:
```
USE_STDPERIPH_DRIVER
AT32F423K8U7_4
```

### 3. Configuration Options

Edit `src/app/modbus_config.h` to configure:

| Parameter | Default | Description |
|-----------|---------|-------------|
| MB_SLAVE_ID_DEFAULT | 1 | Default slave address (1-247) |
| MB_UART_BAUDRATE_DEFAULT | 9600 | Default baud rate |
| MB_UART_PARITY_DEFAULT | 2 | Parity: 0=None, 1=Odd, 2=Even |
| MB_T35_TIMEOUT_MS | 10 | T3.5 timeout in ms |
| MB_BUFFER_SIZE | 256 | TX/RX buffer size |
| MB_SUPPORT_BROADCAST | 1 | Enable broadcast support |
| MB_ENABLE_STATS | 1 | Enable statistics |
| MB_DEBUG_ENABLE | 0 | Enable debug output |

### 4. Basic Usage

#### 4.1 Initialization

```c
#include "modbus_slave.h"
#include "modbus_data.h"
#include "modbus_config.h"
#include "hal_uart.h"
#include "hal_timer.h"

/* Data buffers */
static uint8_t coils_buffer[128];
static uint8_t discrete_inputs_buffer[128];
static uint16_t holding_registers_buffer[256];
static uint16_t input_registers_buffer[256];

/* Callback function example */
static void on_holding_reg_written(uint16_t addr, uint16_t value)
{
    /* Handle register write */
    if (addr == 0) {
        /* Control register */
    }
}

static uint16_t on_input_reg_read(uint16_t addr)
{
    /* Return sensor data */
    switch (addr) {
        case 0: return read_temperature();
        case 1: return read_humidity();
        default: return input_registers_buffer[addr];
    }
}

void modbus_init(void)
{
    /* Register HAL drivers */
    port_uart_at32_register();
    port_timer_at32_register();

    /* Configure data areas */
    mb_data_config_t data_cfg = {
        .coils = coils_buffer,
        .coils_size = 1024,
        .write_coil_cb = NULL,  /* Use buffer directly */

        .discrete_inputs = discrete_inputs_buffer,
        .discrete_inputs_size = 1024,
        .read_discrete_input_cb = NULL,

        .holding_registers = holding_registers_buffer,
        .holding_registers_size = 256,
        .write_holding_reg_cb = on_holding_reg_written,

        .input_registers = input_registers_buffer,
        .input_registers_size = 256,
        .read_input_reg_cb = on_input_reg_read
    };

    modbus_data_init(&data_cfg);

    /* Initialize slave */
    modbus_slave_init(MB_SLAVE_ID_DEFAULT);

    /* Initialize hardware */
    hal_uart_init(MB_UART_BAUDRATE_DEFAULT, HAL_UART_PARITY_EVEN);
    hal_timer_init(MB_T35_TIMEOUT_MS);
}
```

#### 4.2 Main Loop

```c
int main(void)
{
    modbus_init();

    while (1) {
        modbus_slave_poll();

        /* Application tasks */
        update_sensor_data();
    }

    return 0;
}
```

### 5. Data Area Configuration

#### 5.1 Using Buffer Mode (Simple)

The simplest way is to use static buffers. The protocol stack reads/writes buffers directly.

```c
mb_data_config_t data_cfg = {
    .coils = coils_buffer,
    .coils_size = 1024,
    .read_coil_cb = NULL,
    .write_coil_cb = NULL,
    // ... other fields
};
```

#### 5.2 Using Callback Mode (Flexible)

Use callbacks when you need to:
- Read from hardware (sensors, ADC)
- Control hardware (relays, DAC)
- Implement virtual registers
- Add validation logic

```c
/* Read callback - return data */
static uint16_t on_input_reg_read(uint16_t addr)
{
    return read_adc_value(addr);
}

/* Write callback - handle action */
static void on_coil_written(uint16_t addr, uint8_t value)
{
    set_relay(addr, value);
}

mb_data_config_t data_cfg = {
    .input_registers = NULL,  /* No buffer */
    .input_registers_size = 0,
    .read_input_reg_cb = on_input_reg_read,

    .coils = coils_buffer,
    .coils_size = 16,
    .write_coil_cb = on_coil_written,
    // ... other fields
};
```

### 6. Register Mapping Example

| Address | Type | Description | Access |
|---------|------|-------------|--------|
| 0 | Coil | Relay 1 | R/W |
| 1 | Coil | Relay 2 | R/W |
| 0 | Discrete Input | Limit Switch 1 | R/O |
| 1 | Discrete Input | Limit Switch 2 | R/O |
| 0 | Holding Register | Control Word | R/W |
| 1 | Holding Register | Setpoint | R/W |
| 0 | Input Register | Temperature (0.1C) | R/O |
| 1 | Input Register | Humidity (0.1%) | R/O |

### 7. Testing with Modbus Poll

You can use Modbus Poll software to test:

1. Configure connection: 9600, 8N1 (8 data bits, No parity, 1 stop bit)
2. Set slave address: 1
3. Read coils: Function 01, Address 0, Quantity 10
4. Read registers: Function 03, Address 0, Quantity 10

### 8. Troubleshooting

#### No Response
- Check RS485 wiring (A+, B-)
- Verify baud rate and parity settings
- Check slave address matches

#### CRC Error
- Check for electrical noise
- Verify baud rate matches master
- Check cable length and termination

#### Timeout
- Increase T3.5 timeout value
- Check for frame gaps in transmission

### 9. Performance Notes

- Interrupt-driven reception ensures timely byte capture
- T3.5 timer detects frame boundaries automatically
- Response is sent immediately after frame processing
- Typical response time: < 10ms @ 9600 baud

### 10. Memory Usage

| Component | RAM | Flash |
|-----------|-----|-------|
| Protocol Stack | ~1.3KB | ~8KB |
| Data Buffers | Configurable | - |
| HAL Layer | ~100B | ~2KB |

---

## Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.1 | 2026-03-18 | Updated HAL layer API documentation |
| 1.0 | 2026-03-17 | Initial release |
