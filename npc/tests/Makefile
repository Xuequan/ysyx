CFLAGS += -O2 -MMD -Wall -Werror \
		-fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
		-Wno-main -fvisibility=hidden \
		-fdata-sections -ffunction-sections \
		-march=rv32e -mabi=ilp32e



T_PATH = /home/chuan/ysyx-workbench/npc/tests/
T_OBJ = $(T_PATH)/char-test.bin
SRC = $(T_PATH)/char-test.c
ASM = $(T_PATH)/char-test.txt
TEMP = $(T_PATH)/char-test.o

CC 			= riscv64-linux-gnu-gcc
OBJDUMP = riscv64-linux-gnu-objdump
OBJCOPY  = riscv64-linux-gnu-objcopy

$(TEMP) : $(SRC)
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $<

$(T_OBJ) : $(TEMP)
	@$(OBJDUMP) -d $< > $(ASM)
	@$(OBJCOPY) -S -O binary $< $@

run : $(T_OBJ)

clean:
	@rm -f $(T_OBJ) $(ASM) $(TEMP)

.PHONY: run clean


