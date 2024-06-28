TOPNAME = top
# NXDC_FILES = constr/top.nxdc
INC_PATH ?= 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc -O3 \
	--x-assign fast --x-initial fast --noassert
VERILATOR_CFLAGS += --trace
# VERILATOR_CFLAGS += -prof-cfuncs -CFLAGS -DVL_DEBUG

# in order to '`include "ysyx_23060208_npc.h" '
VERILATOR_CFLAGS += -I$(shell find $(abspath ./vsrc))

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

# llvm-config-14 -cxxflags: print the C++ compiler flags needed to use LLVM headers
CXXFLAGS += $(shell llvm-config-14 --cxxflags)
# llvm-config-14 --libs: print all the libraries needed to link against the specified LLVM components
LIBS += $(shell llvm-config-14 --libs)

LDFLAGS += -lasan
LDFLAGS += -lreadline
LDFLAGS += $(LIBS)

LDFLAGS += $(shell llvm-config-14 --ldflags)

#$(BIN): $(VSRCS) $(CSRCS)
$(BIN): $(CSRCS)
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
	@echo 1-----$(shell llvm-config-14 --cxxflags)
	@echo 2-----$(shell llvm-config-14 --libs)
	@echo "================"
