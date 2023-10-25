all:
	@echo "Write this Makefile by your self."

VERILATOR = verilator
VERILATOR_CFLAGS += --cc --exe --trace --build 
OBJ_DIR = ./obj_dir
CSRCS = ./csrc/sim_main.cpp
VSRCS = ./vsrc/top.v

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	$(VERILATOR) $(VERILATOR_CFLAGS) $(CSRCS) $(VSRCS)	
	obj_dir/Vtop
	

include ../Makefile
.PHONY: clean
clean:
	-@rm -r ./obj_dir
