all:
	@echo "Write this Makefile by your self."

TOPNAME = top
INC_PATH ?= 

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)
$(shell mkdir -p $(BUILD_DIR))
CSRCS = ./csrc/sim_main.cpp
VSRCS = ./vsrc/top.v

# rules for NVboard
include $(NVBOARD_HOME)/scripts/nvboard.mk


INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

VERILATOR = verilator
VERILATOR_CFLAGS += --build -cc -O3 --x-assign fast --x-initial fast --assert
$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS)  \
		--top-module $(TOPNAME) $^ \ 
		$(addprefix -CFLAGS, $(CXXFLAGS)) \
		$(addprefix -LDFLAGS, $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) \
		--exe -o $(abspath $(BIN))

sim: $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	-@rf -rf $(OBJ_DIR)
	@echo "===== verilator end ====="
	$(BIN)
	
include ../Makefile

.PHONY: clean
clean:
	-@rm -rf $(BUILD_DIR)
