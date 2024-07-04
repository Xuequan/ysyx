TOPNAME = ysyxSoCFull
VERILATOR = verilator

PERIP_DIR = /home/chuan/ysyx-workbench/ysyxSoC/perip
SOC_DIR = /home/chuan/ysyx-workbench/ysyxSoC/build
NPC_VSRC_DIR = /home/chuan/ysyx-workbench/npc/vsrc

INC_PATH ?=

VERILATOR_CFLAGS += -MMD --build -cc -O3 \
	--x-assign fast --x-initial fast --noassert
VERILATOR_CFLAGS += --trace

# 最后打开这个检查全面
# VERILATOR_CFLAGS += --Wall 
VERILATOR_CFLAGS += --fno-table

# 若出现 active region did not converge, 下面可以辅助debug
# VERILATOR_CFLAGS += -prof-cfuncs -CFLAGS -DVL_DEBUG

# in order to '`include "ysyx_23060208_npc.h" '
VERILATOR_CFLAGS += -I$(NPC_VSRC_DIR)
#VERILATOR_CFLAGS += -I$(SOC_DIR/build)

VERILATOR_CFLAGS += -I$(PERIP_DIR)/uart16550/rtl
VERILATOR_CFLAGS += -I$(PERIP_DIR)/spi/rtl

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

VCD_FILE = dump.vcd

$(shell mkdir -p $(BUILD_DIR))

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt

# project source
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

VSRCS = $(shell find $(NPC_VSRC_DIR) -name "*.v")
VSRCS += $(shell find $(PERIP_DIR) -name "*.v")
VSRCS += $(shell find $(SOC_DIR) -name "*.v")
# rules for NVboard
# include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
CXXFLAGS += -g 

# llvm-config-14 -cxxflags: print the C++ compiler flags needed to use LLVM headers
CXXFLAGS += $(shell llvm-config-14 --cxxflags)
# llvm-config-14 --libs: print all the libraries needed to link against the specified LLVM components
LIBS += $(shell llvm-config-14 --libs)

LDFLAGS += -lasan
LDFLAGS += -lreadline
LDFLAGS += $(LIBS)

LDFLAGS += $(shell llvm-config-14 --ldflags)


# ==================== try start ========================
TEST_CC = riscv64-linux-gnu-gcc
TEST_LD = riscv64-linux-gnu-ld
TEST_OBJDUMP = riscv64-linux-gnu-objdump
TEST_OBJCOPY = riscv64-linux-gnu-objcopy

TEST_FLAGS += -O2 -MMD -Wall -Werror
TEST_FLAGS += -mstrict-align -fdata-sections -ffunction-sections
TEST_FLAGS += -fno-builtin -Wno-main -fno-pic 
# TEST_FLAGS += -march=rv32im_zicsr -mabi=ilp32e
		#	\ -march=rv32e -mabi=ilp32e 
TEST_FLAGS += -mcmodel=medany 
TEST_FLAGS += -static

# TEST_LDFLAGS += -T $(AM_HOME)/scripts/linker.ld  --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
# TEST_LDFLAGS += -e _start 
TEST_LDFLAGS += -melf64lriscv

TEST_IMAGE_REL = build/riscv32e-npc-test
TEST_IMAGE = $(abspath $(TEST_IMAGE_REL))
TEST_FILE = ./tests/char-test.c
TEST_FILE_O = ./tests/test.o

# get .o file
$(TEST_FILE_O): $(TEST_FILE)
	@$(TEST_CC) -std=gnu11 $(TEST_FLAGS) -c -o $@ $<

# get .elf file
$(TEST_IMAGE).elf: $(TEST_FILE_O)
	@$(TEST_LD) $(TEST_LDFLAGS) -o $(TEST_IMAGE).elf  $<

# 最后得到 $(TEST_IMAGE).bin
image: $(TEST_IMAGE).elf
	@$(TEST_OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(TEST_IMAGE).elf $(TEST_IMAGE).bin
 
# ==================== try end ========================

#$(BIN): $(CSRCS)
$(BIN): $(VSRCS) $(CSRCS) 
	@echo "==== inside verilator makefile ========"
	@echo INC_PATH=$(INC_PATH)
	@echo INCFLAGS=$(INCFLAGS)
	@echo VSRCS=$(VSRCS)
	@echo CSRCS=$(CSRCS)
	@echo SOC_DIR=$(SOC_DIR)
	@echo PERIP_DIR=$(PERIP_DIR)
	@echo "==== inside verilator makefile ========"
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		--timescale "1ns/1ns"  --no-timing  \
		$(addprefix -CFLAGS , $(CXXFLAGS))  \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) 

default: sim

NPC_EXEC := $(BIN) $(ARGS) $(IMG) $(TEST_IMAGE).bin

sim-env: $(BIN)

sim: sim-env image
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo ========inside npc/makefile ==========
	@echo ARGS=$(ARGS)
	@echo IMG=$(IMG)
	@echo NPC_EXEC=$(NPC_EXEC)
	@echo LIBS=$(LIBS)
	@echo VSRCS=$(VSRCS)
	@echo here $(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) 
	@echo llvm-config-14 --cxxflags = $(shell llvm-confag-11 --cxxflags)
	@echo llvm-config-14 --ldflags = $(shell llvm-config-14 --ldflags)
	@echo llvm-config-14 --libs = $(shell llvm-config-14 --libs)
	@echo =======================================
	$(NPC_EXEC)
	
include ../Makefile

gdb: $(BIN)
	#gdb -s $^ --args $(IMG) $^
	#gdb -s $(NPC_EXEC)
	gdb -s $^ --args $(ARGS)

wave:
	gtkwave $(VCD_FILE)

clean:
	-@rm -rf $(BUILD_DIR)
	-@rm $(VCD_FILE)

.PHONY: clean default wave sim
