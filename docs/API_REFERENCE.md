# Modbus RTU Slave API Reference

## Table of Contents

1. [Modbus Slave API](#1-modbus-slave-api)
2. [Modbus Data API](#2-modbus-data-api)
3. [HAL UART API](#3-hal-uart-api)
4. [HAL Timer API](#4-hal-timer-api)
5. [Data Structures](#5-data-structures)
6. [Error Codes](#6-error-codes)
7. [Exception Codes](#7-exception-codes)

---

## 1. Modbus Slave API

### modbus_slave_init

**Description:** Initialize Modbus slave

**Prototype:**
```c
int modbus_slave_init(uint8_t slave_id);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| slave_id | uint8_t | Slave address (1-247) |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Invalid slave address |

**Example:**
```c
modbus_slave_init(1);  // Set slave address to 1
```

---

### modbus_slave_deinit

**Description:** Deinitialize Modbus slave

**Prototype:**
```c
void modbus_slave_deinit(void);
```

**Example:**
```c
modbus_slave_deinit();
```

---

### modbus_slave_poll

**Description:** Poll function for state machine (call in main loop)

**Prototype:**
```c
void modbus_slave_poll(void);
```

**Example:**
```c
while (1) {
    modbus_slave_poll();
}
```

---

### modbus_slave_set_address

**Description:** Change slave address at runtime

**Prototype:**
```c
void modbus_slave_set_address(uint8_t slave_id);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| slave_id | uint8_t | New slave address (1-247) |

**Example:**
```c
modbus_slave_set_address(10);
```

---

### modbus_slave_get_address

**Description:** Get current slave address

**Prototype:**
```c
uint8_t modbus_slave_get_address(void);
```

**Return Value:** Current slave address

**Example:**
```c
uint8_t addr = modbus_slave_get_address();
```

---

### modbus_slave_get_state

**Description:** Get current state machine state

**Prototype:**
```c
mb_state_t modbus_slave_get_state(void);
```

**Return Value:**
| State | Description |
|-------|-------------|
| MB_STATE_IDLE | Idle, waiting for request |
| MB_STATE_RECEIVING | Receiving frame |
| MB_STATE_PROCESSING | Processing request |
| MB_STATE_RESPONDING | Sending response |
| MB_STATE_ERROR | Error state |

---

### modbus_slave_set_rx_callback

**Description:** Set frame received callback

**Prototype:**
```c
void modbus_slave_set_rx_callback(mb_callback_t callback);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| callback | mb_callback_t | Callback function (void (*)(void)) |

**Example:**
```c
void on_frame_received(void) {
    // Frame received
}

modbus_slave_set_rx_callback(on_frame_received);
```

---

### modbus_slave_set_tx_callback

**Description:** Set transmission complete callback

**Prototype:**
```c
void modbus_slave_set_tx_callback(mb_callback_t callback);
```

**Example:**
```c
void on_frame_sent(void) {
    // Response sent
}

modbus_slave_set_tx_callback(on_frame_sent);
```

---

### modbus_slave_set_error_callback

**Description:** Set error callback

**Prototype:**
```c
void modbus_slave_set_error_callback(mb_error_cb_t callback);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| callback | mb_error_cb_t | Callback function (void (*)(uint8_t error)) |

**Example:**
```c
void on_error(uint8_t error) {
    printf("Error: %d\n", error);
}

modbus_slave_set_error_callback(on_error);
```

---

### modbus_slave_rx_byte_isr

**Description:** Inject received byte (for testing)

**Prototype:**
```c
void modbus_slave_rx_byte_isr(uint8_t byte);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| byte | uint8_t | Received byte |

---

### modbus_slave_t35_expired_isr

**Description:** Signal T3.5 timeout (for testing)

**Prototype:**
```c
void modbus_slave_t35_expired_isr(void);
```

---

### modbus_slave_get_stats

**Description:** Get communication statistics

**Prototype:**
```c
void modbus_slave_get_stats(mb_stats_t *stats);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| stats | mb_stats_t* | Statistics structure pointer |

**Example:**
```c
mb_stats_t stats;
modbus_slave_get_stats(&stats);
printf("RX: %lu, TX: %lu, Errors: %lu\n",
       stats.rx_count, stats.tx_count, stats.error_count);
```

---

### modbus_slave_reset_stats

**Description:** Reset statistics counters

**Prototype:**
```c
void modbus_slave_reset_stats(void);
```

---

## 2. Modbus Data API

### modbus_data_init

**Description:** Initialize data area configuration

**Prototype:**
```c
int modbus_data_init(const mb_data_config_t *config);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| config | mb_data_config_t* | Data configuration structure |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Invalid parameter |

**Example:**
```c
mb_data_config_t data_cfg = {
    .coils = coils_buffer,
    .coils_size = 1024,
    .read_coil_cb = NULL,
    .write_coil_cb = on_coil_written,
    // ... other fields
};
modbus_data_init(&data_cfg);
```

---

### modbus_data_read_coil

**Description:** Read coil value

**Prototype:**
```c
int modbus_data_read_coil(uint16_t addr, uint8_t *value);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| addr | uint16_t | Coil address |
| value | uint8_t* | Output value (0 or 1) |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Address out of range |

---

### modbus_data_write_coil

**Description:** Write coil value

**Prototype:**
```c
int modbus_data_write_coil(uint16_t addr, uint8_t value);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| addr | uint16_t | Coil address |
| value | uint8_t | Value to write (0 or 1) |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Address out of range |

---

### modbus_data_read_holding_reg

**Description:** Read holding register value

**Prototype:**
```c
int modbus_data_read_holding_reg(uint16_t addr, uint16_t *value);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| addr | uint16_t | Register address |
| value | uint16_t* | Output value |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Address out of range |

---

### modbus_data_write_holding_reg

**Description:** Write holding register value

**Prototype:**
```c
int modbus_data_write_holding_reg(uint16_t addr, uint16_t value);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| addr | uint16_t | Register address |
| value | uint16_t | Value to write |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Address out of range |

---

## 3. HAL UART API

### hal_uart_init

**Description:** Initialize UART

**Prototype:**
```c
int hal_uart_init(uint32_t baudrate, hal_uart_parity_t parity);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| baudrate | uint32_t | Baud rate (e.g., 9600, 19200) |
| parity | hal_uart_parity_t | Parity setting |

**Parity Values:**
| Value | Description |
|-------|-------------|
| HAL_UART_PARITY_NONE | No parity |
| HAL_UART_PARITY_ODD | Odd parity |
| HAL_UART_PARITY_EVEN | Even parity |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Error |

---

### hal_uart_deinit

**Description:** Deinitialize UART

**Prototype:**
```c
void hal_uart_deinit(void);
```

---

### hal_uart_send

**Description:** Send data asynchronously

**Prototype:**
```c
int hal_uart_send(const uint8_t *data, uint16_t len);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| data | const uint8_t* | Data buffer |
| len | uint16_t | Data length |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Busy or error |

---

### hal_uart_set_rx_callback

**Description:** Set byte received callback

**Prototype:**
```c
void hal_uart_set_rx_callback(void (*callback)(uint8_t byte));
```

---

### hal_uart_set_tx_complete_callback

**Description:** Set transmission complete callback

**Prototype:**
```c
void hal_uart_set_tx_complete_callback(void (*callback)(void));
```

---

### hal_uart_register

**Description:** Register HAL UART driver

**Prototype:**
```c
void hal_uart_register(const hal_uart_t *uart);
```

---

## 4. HAL Timer API

### hal_timer_init

**Description:** Initialize timer

**Prototype:**
```c
int hal_timer_init(uint32_t timeout_ms);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| timeout_ms | uint32_t | Timeout in milliseconds |

**Return Value:**
| Value | Description |
|-------|-------------|
| 0 | Success |
| -1 | Error |

---

### hal_timer_deinit

**Description:** Deinitialize timer

**Prototype:**
```c
void hal_timer_deinit(void);
```

---

### hal_timer_start

**Description:** Start timer

**Prototype:**
```c
void hal_timer_start(void);
```

---

### hal_timer_stop

**Description:** Stop timer

**Prototype:**
```c
void hal_timer_stop(void);
```

---

### hal_timer_reset

**Description:** Reset timer counter

**Prototype:**
```c
void hal_timer_reset(void);
```

---

### hal_timer_set_timeout

**Description:** Set timeout value

**Prototype:**
```c
void hal_timer_set_timeout(uint32_t timeout_ms);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| timeout_ms | uint32_t | Timeout in milliseconds |

---

### hal_timer_set_callback

**Description:** Set timeout callback

**Prototype:**
```c
void hal_timer_set_callback(void (*callback)(void));
```

---

### hal_timer_is_expired

**Description:** Check if timer expired (polling mode)

**Prototype:**
```c
bool hal_timer_is_expired(void);
```

**Return Value:**
| Value | Description |
|-------|-------------|
| true | Timer expired |
| false | Timer not expired |

---

### hal_timer_register

**Description:** Register HAL timer driver

**Prototype:**
```c
void hal_timer_register(const hal_timer_t *timer);
```

---

## 5. Data Structures

### mb_data_config_t

**Description:** Data area configuration

```c
typedef struct {
    /* Coils */
    uint8_t *coils;
    uint16_t coils_size;
    uint8_t (*read_coil_cb)(uint16_t addr);
    void (*write_coil_cb)(uint16_t addr, uint8_t value);

    /* Discrete Inputs */
    uint8_t *discrete_inputs;
    uint16_t discrete_inputs_size;
    uint8_t (*read_discrete_input_cb)(uint16_t addr);

    /* Holding Registers */
    uint16_t *holding_registers;
    uint16_t holding_registers_size;
    uint16_t (*read_holding_reg_cb)(uint16_t addr);
    void (*write_holding_reg_cb)(uint16_t addr, uint16_t value);

    /* Input Registers */
    uint16_t *input_registers;
    uint16_t input_registers_size;
    uint16_t (*read_input_reg_cb)(uint16_t addr);
} mb_data_config_t;
```

---

### mb_stats_t

**Description:** Communication statistics

```c
typedef struct {
    uint32_t rx_count;          /* Received frames */
    uint32_t tx_count;          /* Transmitted frames */
    uint32_t error_count;       /* Total errors */
    uint32_t crc_error_count;   /* CRC errors */
    uint32_t broadcast_count;   /* Broadcast frames */
} mb_stats_t;
```

---

### mb_state_t

**Description:** State machine states

```c
typedef enum {
    MB_STATE_IDLE = 0,
    MB_STATE_RECEIVING,
    MB_STATE_PROCESSING,
    MB_STATE_RESPONDING,
    MB_STATE_ERROR
} mb_state_t;
```

---

### hal_uart_t

**Description:** UART HAL interface

```c
typedef struct {
    int (*init)(uint32_t baudrate, hal_uart_parity_t parity);
    void (*deinit)(void);
    int (*send)(const uint8_t *data, uint16_t len);
    void (*set_rx_callback)(void (*callback)(uint8_t byte));
    void (*set_tx_complete_callback)(void (*callback)(void));
} hal_uart_t;
```

---

### hal_timer_t

**Description:** Timer HAL interface

```c
typedef struct {
    int (*init)(uint32_t timeout_ms);
    void (*deinit)(void);
    void (*start)(void);
    void (*stop)(void);
    void (*reset)(void);
    void (*set_timeout)(uint32_t timeout_ms);
    void (*set_callback)(void (*callback)(void));
    bool (*is_expired)(void);
} hal_timer_t;
```

---

## 6. Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | MB_ERROR_NONE | No error |
| 1 | MB_ERROR_CRC | CRC error |
| 2 | MB_ERROR_FRAME | Frame error |
| 3 | MB_ERROR_BUFFER_OVERFLOW | Buffer overflow |
| 4 | MB_ERROR_INVALID_FUNCTION | Invalid function code |
| 5 | MB_ERROR_INVALID_ADDRESS | Invalid address |
| 6 | MB_ERROR_INVALID_VALUE | Invalid value |
| 7 | MB_ERROR_SLAVE_BUSY | Slave busy |

---

## 7. Exception Codes

| Code | Name | Description |
|------|------|-------------|
| 0x01 | MB_EX_ILLEGAL_FUNCTION | Illegal function code |
| 0x02 | MB_EX_ILLEGAL_ADDRESS | Illegal data address |
| 0x03 | MB_EX_ILLEGAL_VALUE | Illegal data value |
| 0x04 | MB_EX_SLAVE_FAILURE | Slave device failure |
| 0x05 | MB_EX_ACKNOWLEDGE | Acknowledge (long operation) |
| 0x06 | MB_EX_SLAVE_BUSY | Slave device busy |
| 0x08 | MB_EX_MEMORY_PARITY | Memory parity error |

---

## Callback Types

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
    HAL_UART_PARITY_NONE = 0,
    HAL_UART_PARITY_ODD = 1,
    HAL_UART_PARITY_EVEN = 2
} hal_uart_parity_t;
```

---

## Porting Guide

### Step 1: Create Port Files

Create `port_uart_xxx.c` and `port_timer_xxx.c` for your platform.

### Step 2: Implement HAL Interface

Implement all required functions:

**UART:**
- `init` - Initialize UART with baudrate and parity
- `deinit` - Deinitialize UART
- `send` - Send data asynchronously
- `set_rx_callback` - Set RX callback
- `set_tx_complete_callback` - Set TX complete callback

**Timer:**
- `init` - Initialize timer with timeout
- `deinit` - Deinitialize timer
- `start` - Start timer
- `stop` - Stop timer
- `reset` - Reset counter
- `set_timeout` - Set timeout value
- `set_callback` - Set timeout callback
- `is_expired` - Check expiration (optional)

### Step 3: Register HAL

```c
void port_uart_xxx_register(void) {
    hal_uart_register(&hal_uart_xxx);
}

void port_timer_xxx_register(void) {
    hal_timer_register(&hal_timer_xxx);
}
```

---

## Version History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2026-03-17 | Initial release |
