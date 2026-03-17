# Modbus RTU从站协议栈 Makefile
# 作者: Claude
# 日期: 2026-03-17

# ============================================================================
# 编译器配置
# ============================================================================

CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# ============================================================================
# 目录配置
# ============================================================================

PROJECT_NAME = modbus_slave

SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# ============================================================================
# 源文件
# ============================================================================

# 核心源文件
C_SOURCES = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/app/modbus_slave.c \
	$(SRC_DIR)/app/modbus_data.c \
	$(SRC_DIR)/protocol/modbus_rtu.c \
	$(SRC_DIR)/protocol/modbus_crc.c \
	$(SRC_DIR)/protocol/modbus_pdu.c \
	$(SRC_DIR)/hal/hal_uart.c \
	$(SRC_DIR)/hal/hal_timer.c \
	$(SRC_DIR)/port/port_uart_stm32.c \
	$(SRC_DIR)/port/port_timer_stm32.c

# ============================================================================
# 头文件路径
# ============================================================================

C_INCLUDES = \
	-I$(INC_DIR) \
	-I$(SRC_DIR) \
	-I$(SRC_DIR)/app \
	-I$(SRC_DIR)/protocol \
	-I$(SRC_DIR)/hal \
	-I$(SRC_DIR)/port

# ============================================================================
# 编译选项
# ============================================================================

# MCU型号（根据实际情况修改）
MCU = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# 编译标志
CFLAGS = $(MCU) $(C_INCLUDES) -O2 -g -Wall -Wextra -Werror
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -std=c99

# 链接标志
LDFLAGS = $(MCU) -specs=nano.specs -T stm32_flash.ld
LDFLAGS += -Wl,-Map=$(BIN_DIR)/$(PROJECT_NAME).map
LDFLAGS += -Wl,--gc-sections

# 宏定义
C_DEFS = -DSTM32F407xx -DUSE_HAL_DRIVER

# ============================================================================
# 目标文件
# ============================================================================

OBJECTS = $(addprefix $(OBJ_DIR)/, $(C_SOURCES:.c=.o))

# ============================================================================
# 构建规则
# ============================================================================

.PHONY: all clean size

all: $(BIN_DIR)/$(PROJECT_NAME).elf $(BIN_DIR)/$(PROJECT_NAME).hex $(BIN_DIR)/$(PROJECT_NAME).bin

# 创建目录
$(BUILD_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@
	mkdir -p $(OBJ_DIR)/app
	mkdir -p $(OBJ_DIR)/protocol
	mkdir -p $(OBJ_DIR)/hal
	mkdir -p $(OBJ_DIR)/port

$(BIN_DIR):
	mkdir -p $@

# 编译C文件
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@echo "CC $<"
	$(CC) -c $(CFLAGS) $(C_DEFS) -Wa,-a,-ad,-alms=$(@:.o=.lst) $< -o $@

# 链接
$(BIN_DIR)/$(PROJECT_NAME).elf: $(OBJECTS) | $(BIN_DIR)
	@echo "LD $@"
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	@echo ""
	@$(SIZE) $@

# 生成HEX文件
$(BIN_DIR)/$(PROJECT_NAME).hex: $(BIN_DIR)/$(PROJECT_NAME).elf
	@echo "OBJCOPY $@"
	$(OBJCOPY) -O ihex $< $@

# 生成BIN文件
$(BIN_DIR)/$(PROJECT_NAME).bin: $(BIN_DIR)/$(PROJECT_NAME).elf
	@echo "OBJCOPY $@"
	$(OBJCOPY) -O binary -S $< $@

# 清理
clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)
	@echo "Done."

# 显示大小
size: $(BIN_DIR)/$(PROJECT_NAME).elf
	$(SIZE) $<

# ============================================================================
# 调试和烧录（需要根据实际工具配置）
# ============================================================================

# 使用OpenOCD烧录
flash: $(BIN_DIR)/$(PROJECT_NAME).elf
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg \
		-c "program $< verify reset exit"

# 使用GDB调试
debug: $(BIN_DIR)/$(PROJECT_NAME).elf
	arm-none-eabi-gdb -ex "target extended-remote localhost:3333" $<

# ============================================================================
# PC平台测试编译（不使用ARM工具链）
# ============================================================================

PC_CC = gcc
PC_CFLAGS = -O2 -g -Wall -Wextra -std=c99
PC_CFLAGS += $(C_INCLUDES)
PC_CFLAGS += -DMB_DEBUG_ENABLE=1 -DMB_ENABLE_STATS=1

PC_SOURCES = \
	$(SRC_DIR)/app/modbus_slave.c \
	$(SRC_DIR)/app/modbus_data.c \
	$(SRC_DIR)/protocol/modbus_rtu.c \
	$(SRC_DIR)/protocol/modbus_crc.c \
	$(SRC_DIR)/protocol/modbus_pdu.c \
	$(SRC_DIR)/hal/hal_uart.c \
	$(SRC_DIR)/hal/hal_timer.c

# PC平台测试目标
pc-test:
	@echo "Building PC test..."
	$(PC_CC) $(PC_CFLAGS) -c $(SRC_DIR)/app/modbus_slave.c -o $(BUILD_DIR)/modbus_slave.o
	$(PC_CC) $(PC_CFLAGS) -c $(SRC_DIR)/app/modbus_data.c -o $(BUILD_DIR)/modbus_data.o
	$(PC_CC) $(PC_CFLAGS) -c $(SRC_DIR)/protocol/modbus_rtu.c -o $(BUILD_DIR)/modbus_rtu.o
	$(PC_CC) $(PC_CFLAGS) -c $(SRC_DIR)/protocol/modbus_crc.c -o $(BUILD_DIR)/modbus_crc.o
	$(PC_CC) $(PC_CFLAGS) -c $(SRC_DIR)/protocol/modbus_pdu.c -o $(BUILD_DIR)/modbus_pdu.o
	$(PC_CC) $(PC_CFLAGS) -c $(SRC_DIR)/hal/hal_uart.c -o $(BUILD_DIR)/hal_uart.o
	$(PC_CC) $(PC_CFLAGS) -c $(SRC_DIR)/hal/hal_timer.c -o $(BUILD_DIR)/hal_timer.o
	@echo "PC test build complete."

# ============================================================================
# 帮助信息
# ============================================================================

help:
	@echo "Modbus RTU从站协议栈构建系统"
	@echo ""
	@echo "使用方法:"
	@echo "  make          - 构建项目（ARM目标）"
	@echo "  make clean    - 清理构建文件"
	@echo "  make size     - 显示程序大小"
	@echo "  make flash    - 烧录到目标板"
	@echo "  make debug    - 启动GDB调试"
	@echo "  make pc-test  - 构建PC平台测试"
	@echo "  make help     - 显示此帮助信息"
	@echo ""
