# define top name
TOPNAME = ysyxSoCFull
VERILATOR = verilator

# define some source directory name
PERIP_DIR = /home/chuan/ysyx-workbench/ysyxSoC/perip
SOC_DIR = /home/chuan/ysyx-workbench/ysyxSoC/build
NPC_VSRC_DIR = /home/chuan/ysyx-workbench/npc/vsrc
INC_PATH ?=

# define verilator flags
VERILATOR_CFLAGS += -MMD --build -cc -O3 \
	--x-assign fast --x-initial fast --noassert --trace --autoflush
# in order to '`include "ysyx_23060208_npc.h" '
VERILATOR_CFLAGS += -I$(NPC_VSRC_DIR)
VERILATOR_CFLAGS += -I$(PERIP_DIR)/uart16550/rtl
VERILATOR_CFLAGS += -I$(PERIP_DIR)/spi/rtl

# define build_dir 
BUILD_DIR = ./build
$(shell mkdir -p $(BUILD_DIR))
OBJ_DIR = $(BUILD_DIR)/obj_dir

# this is the execute file at the end
TOP_BIN = $(BUILD_DIR)/$(TOPNAME)

# wave file
VCD_FILE = dump.vcd

# override log file
override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt

# project c++ sorce file
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

# project verilog sorce file
VSRCS = $(shell find $(NPC_VSRC_DIR) -name "*.v")
VSRCS += $(shell find $(PERIP_DIR) -name "*.v")
VSRCS += $(shell find $(SOC_DIR) -name "*.v")

## nvboard
# top.nxdc which helps to bind pins
NXDC_FILES = nvboard/top.nxdc

# generate function auto_bind.cpp which will be called in sim.cpp
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# incflags for compiling simmulation c++ file
INCFLAGS = $(addprefix -I, $(INC_PATH))

# cxxcflags for compiling simmulation c++ file
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
# cxxflags for llvm
CXXFLAGS += -I/usr/lib/llvm-14/include -std=c++14 -fno-exceptions
# for libreadline 
LDFLAGS += -lreadline
# libs & ldflags for llvm 
LDFLAGS += $(shell llvm-config-14 --libs) $(shell llvm-config-14 --ldflags)

# rules for NVboard
include $(NVBOARD_HOME)/scripts/nvboard.mk
# nvboard archive file
CSRCS += $(SRC_AUTO_BIND)

# generate execute file
$(TOP_BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@echo "==== inside verilator makefile ========"
	@echo INC_PATH=$(INC_PATH)
	@echo '------------------'
	@echo INCFLAGS=$(INCFLAGS)
	@echo '------------------'
	@echo VSRCS=$(VSRCS)
	@echo '------------------'
	@echo CSRCS=$(CSRCS)
	@echo '------------------'
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo '------------------'
	@echo LDFLASGS=$(LDFLAGS)
	@echo '------------------'
	@echo NVBOARD_ARCHIVE=$(NVBOARD_ARCHIVE)
	@echo "==== inside verilator makefile ========"
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		--timescale "1ns/1ns"  --no-timing  \
		$(addprefix -CFLAGS , $(CXXFLAGS))  \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(TOP_BIN)) 

default: sim

# execute file, plus args, img file from AM 
NPC_EXEC := $(TOP_BIN) $(ARGS) $(IMG)

sim: $(TOP_BIN) 
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo ========npc/makefile =======================
	@echo ARGS=$(ARGS)
	@echo '------------------'
	@echo NPC_EXEC=$(NPC_EXEC)
	@echo '------------------'
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo '------------------'
	@echo LDFLASGS=$(LDFLAGS)
	@echo '------------------'
	@echo $(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(TOP_BIN)) 
	@echo =============echo end==========================
	$(NPC_EXEC)
	
include ../Makefile

wave:
	gtkwave $(VCD_FILE)

clean:
	-@rm -rf $(BUILD_DIR)
	-@rm $(VCD_FILE)

.PHONY: clean default wave sim
