all:
	@echo "Write this Makefile by your self."

OBJ_DIR = ./obj_dir
CSRCS = ./csrc/sim_main.cpp
VSRCS = ./vsrc/top.v

# rules for NVboard
include $(NVBOARD_HOME)/scripts/nvboard.mk

TOPNAME = top
INC_PATH ?= 

INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \                             -O3 --x-assign fast --x-initial fast --noassert

sim: $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@rf -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS, $(CXXFLAGS)) $(addprefix -LDFLAGS, $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe	
	@echo "===== verilator end ====="
	./obj_dir/Vtop
	
include ../Makefile

.PHONY: clean
clean:
	-@rm -rf ./obj_dir
