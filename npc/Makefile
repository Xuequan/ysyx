TOPNAME = ysyxSoCFull
VERILATOR = verilator

PERIP_DIR = /home/chuan/ysyx-workbench/ysyxSoC/perip
SOC_DIR = /home/chuan/ysyx-workbench/ysyxSoC/build
NPC_VSRC_DIR = /home/chuan/ysyx-workbench/npc/vsrc

INC_PATH ?=

VERILATOR_CFLAGS += -MMD --build -cc -O3 \
	--x-assign fast --x-initial fast --noassert
VERILATOR_CFLAGS += --trace
	
VERILATOR_CFLAGS += -fno-table  \

# 最后打开这个检查全面
# VERILATOR_CFLAGS += --Wall 
#
VERILATOR_CFLAGS += --autoflush

# 若出现 active region did not converge, 下面可以辅助debug
# VERILATOR_CFLAGS += -prof-cfuncs -CFLAGS -DVL_DEBUG

# in order to '`include "ysyx_23060208_npc.h" '
VERILATOR_CFLAGS += -I$(NPC_VSRC_DIR)

VERILATOR_CFLAGS += -I$(PERIP_DIR)/uart16550/rtl
VERILATOR_CFLAGS += -I$(PERIP_DIR)/spi/rtl

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
TOP_BIN = $(BUILD_DIR)/$(TOPNAME)

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


CXXFLAGS += $(shell llvm-config-14 --cxxflags)
LIBS += $(shell llvm-config-14 --libs)

LDFLAGS += -lasan
LDFLAGS += -lreadline
LDFLAGS += $(LIBS)

LDFLAGS += $(shell llvm-config-14 --ldflags)

$(TOP_BIN): $(VSRCS) $(CSRCS) 
	@echo "==== inside verilator makefile ========"
	@echo INC_PATH=$(INC_PATH)
	@echo INCFLAGS=$(INCFLAGS)
	@echo VSRCS=$(VSRCS)
	@echo CSRCS=$(CSRCS)
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo LDFLASGS=$(LDFLAGS)
	@echo "==== inside verilator makefile ========"
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		--timescale "1ns/1ns"  --no-timing  \
		$(addprefix -CFLAGS , $(CXXFLAGS))  \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(TOP_BIN)) 

default: sim

NPC_EXEC := $(TOP_BIN) $(ARGS) $(IMG)

sim: $(TOP_BIN) 
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo ========inside npc/makefile ==========
	@echo ARGS=$(ARGS)
	@echo NPC_EXEC=$(NPC_EXEC)
	@echo here $(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(TOP_BIN)) 
	@echo =======================================
	$(NPC_EXEC)
	
include ../Makefile

gdb: $(TOP_BIN)
	gdb -s $^ --args $(ARGS)

wave:
	gtkwave $(VCD_FILE)

clean:
	-@rm -rf $(BUILD_DIR)
	-@rm $(VCD_FILE)

.PHONY: clean default wave sim
