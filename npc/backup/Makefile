TOPNAME = ysyxSoCFull
VERILATOR = verilator

PERIP_DIR = /home/chuan/ysyx-workbench/ysyxSoC/perip
SOC_DIR = /home/chuan/ysyx-workbench/ysyxSoC/build
NPC_VSRC_DIR = /home/chuan/ysyx-workbench/npc/vsrc
VCD_FILE = dump.vcd
BUILD_DIR = ./build
$(shell mkdir -p $(BUILD_DIR))

VERILATOR_CFLAGS += -MMD --build -cc -O3 \
	--x-assign fast --x-initial fast --noassert --trace --autoflush

# in order to '`include "ysyx_23060208_npc.h" '
VERILATOR_CFLAGS += -I$(NPC_VSRC_DIR)
VERILATOR_CFLAGS += -I$(PERIP_DIR)/uart16550/rtl
VERILATOR_CFLAGS += -I$(PERIP_DIR)/spi/rtl

OBJ_DIR = $(BUILD_DIR)/obj_dir
TOP_BIN = $(BUILD_DIR)/$(TOPNAME)

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt

# project source
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
VSRCS = $(shell find $(NPC_VSRC_DIR) -name "*.v")
VSRCS += $(shell find $(PERIP_DIR) -name "*.v")
VSRCS += $(shell find $(SOC_DIR) -name "*.v")

INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
CXXFLAGS += -I/usr/lib/llvm-14/include -std=c++14 -fno-exceptions
LDFLAGS += -lreadline
LDFLAGS += $(shell llvm-config-14 --libs) $(shell llvm-config-14 --ldflags)

$(TOP_BIN): $(VSRCS) $(CSRCS) 
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
	@echo "==== inside verilator makefile end ========"
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
	@echo =============end =======================
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
