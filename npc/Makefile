TOPNAME = top
# NXDC_FILES = constr/top.nxdc
INC_PATH ?= 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc -O3 \
	--x-assign fast --x-initial fast --noassert
VERILATOR_CFLAGS += --trace

VERILATOR_CFLGAS += --debug
VERILATOR_CFLGAS += --gdbbt
# for gdb
VERILATOR_CFLAGS += -CFLAGS -ggdb -LDFLAGS -ggdb -CFLAGS -DVL_DEBUG=1 -CFLAGS -D_GLIBCXX_DEBUG

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

VCD_FILE = dump.vcd

$(shell mkdir -p $(BUILD_DIR))

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

# rules for NVboard
# include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

#$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
$(BIN): $(VSRCS) $(CSRCS)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^  \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))
	@echo "===== verilator end ====="

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
	@echo "================"

default: sim

sim: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo ========inside npc/makefile ==========
	@echo ARGS=$(ARGS)
	@echo IMG=$(IMG)
	@echo $^
	@echo =======================================
	@$^ $(IMG)
	
include ../Makefile

gdb: $(BIN)
	gdb -s $(VERILATOR) --args $(IMG) $^

wave:
	gtkwave $(VCD_FILE)

clean:
	-@rm -rf $(BUILD_DIR)
	-@rm $(VCD_FILE)

.PHONY: clean default wave sim
