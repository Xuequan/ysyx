all:
	@echo "Write this Makefile by your self."

OBJ_DIR = ./obj_dir

# 生成./build 目录，最后auto_bind.cpp 放里头
#BUILD_DIR = ./build
#$(shell mkdir -p $(BUILD_DIR))

#自己编写的 top.nxdc
#NXDC_FILES = ./constr/top.nxdc

# 自动生成 auto_bind.cpp 
#SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
#$(SRC_AUTO_BIND): $(NXDC_FILES)
	#python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

CSRCS = ./csrc/sim_main.cpp
VSRCS = ./vsrc/top.v
# 将 auto_bind.cpp 加入到verilator input 文件
CSRCS += ./auto_bind.cpp

# rules for NVboard
include $(NVBOARD_HOME)/scripts/nvboard.mk

TOPNAME = top
INC_PATH ?= 

INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \                             -O3 --x-assign fast --x-initial fast --noassert

#BIN = $(BUILD_DIR)/$(TOPNAME)
BIN = ./obj_dir/Vtop
$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@echo "===== verilator begin ====="
	@rf -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS, $(CXXFLAGS)) $(addprefix -LDFLAGS, $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))	
	@echo "===== verilator end ====="


sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	$(BIN)
	
include ../Makefile

.PHONY: clean
clean:
	-@rm -r ./obj_dir
