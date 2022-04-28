ARMGNU ?= aarch64-linux-gnu

COPS = -Wall -nostdlib -nostdinc -Iinclude
ASMOPS = -Iinclude  -D__ASSEMBLY__

BUILD_DIR = build
SRC_DIR = src

all : rpios.bin

clean :
	rm -rf $(BUILD_DIR) *.img
run :
	qemu-system-aarch64 -machine raspi3 -nographic -kernel rpios.img

debug : 
	qemu-system-aarch64 -machine raspi3 -nographic -kernel rpios.img -S -s

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

rpios.bin: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/rpios.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/rpios.elf -O binary rpios.img
