all:
	@echo "Write this Makefile by your self."

VERILATOR = verilator
VERILATOR_CFLAGS += --cc --exe --trace --build 
src1 = ./csrc/sim_main.cpp
src2 = ./vsrc/top.v
Vtop: $(src1) $(src2)
	$(VERILATOR) $(VERILATOR_CFLAGS) @^
	#verilator --cc --exe --trace --build -j @^

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	./obj_dir/Vtop
	

include ../Makefile
