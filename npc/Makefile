<<<<<<< HEAD
TOPNAME = top
# NXDC_FILES = constr/top.nxdc
INC_PATH ?= 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc -O3 \
	--x-assign fast --x-initial fast --noassert
VERILATOR_CFLAGS += --trace

#VERILATOR_CFLAGS += --debug
#VERILATOR_CFLAGS += --gdbbt

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

VCD_FILE = dump.vcd

$(shell mkdir -p $(BUILD_DIR))

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

# rules for NVboard
# include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
CXXFLAGS += -g 

# llvm-config -cxxflags: print the C++ compiler flags needed to use LLVM headers
CXXFLAGS += $(shell llvm-config --cxxflags)
# llvm-config --libs: print all the libraries needed to link against the specified LLVM components
LIBS += $(shell llvm-config --libs)

LDFLAGS += -lasan
LDFLAGS += -lreadline
LDFLAGS += $(LIBS)

LDFLAGS += $(shell llvm-config --ldflags)

$(BIN): $(VSRCS) $(CSRCS)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) 

default: sim

NPC_EXEC := $(BIN) $(ARGS) $(IMG)

sim-env: $(BIN)

sim: sim-env
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo ========inside npc/makefile ==========
	@echo ARGS=$(ARGS)
	@echo IMG=$(IMG)
	@echo NPC_EXEC=$(NPC_EXEC)
	@echo LIBS=$(LIBS)
	@echo here $(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) 
	@echo llvm-config --cxxflags = $(shell llvm-config --cxxflags)
	@echo llvm-config --ldflags = $(shell llvm-config --ldflags)
	@echo llvm-config --libs = $(shell llvm-config --libs)
	@echo =======================================
=======
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
>>>>>>> tracer-ysyx
	$(NPC_EXEC)
	
include ../Makefile

<<<<<<< HEAD
gdb: $(BIN)
	#gdb -s $^ --args $(IMG) $^
	#gdb -s $(NPC_EXEC)
	gdb -s $^ --args $(ARGS)

=======
>>>>>>> tracer-ysyx
wave:
	gtkwave $(VCD_FILE)

clean:
	-@rm -rf $(BUILD_DIR)
	-@rm $(VCD_FILE)

.PHONY: clean default wave sim
<<<<<<< HEAD

print:
	@echo "================"
	@echo  $(VERILATOR) $(VERILATOR_CFLAGS)  \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) \
		--exe -o $(abspath $(BIN))
	@echo CFLGAS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo 1-----$(shell llvm-config --cxxflags)
	@echo 2-----$(shell llvm-config --libs)
	@echo "================"
=======
>>>>>>> tracer-ysyx
