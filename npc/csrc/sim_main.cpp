#include <memory>
#include <verilated.h>
#include <Vtop.h>

#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include "verilated_vcd_c.h"

#include "nvboard.h"

void nvboard_bind_all_pins(Vtop *top) {
	nvboard_bind_pin(&top->a, false, true, 1, LD0);
	nvboard_bind_pin(&top->b, false, true, 1, LD0);
	nvboard_bind_pin(&top->f, false, false, 1, LD0);
}

int main(int argc, char** argv) {
	if (false && argc && argv) {}

	const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};

	const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};
	
	nvboard_bind_all_pins(top);
	nvboard_init();

	contextp->debug(0);
	contextp->randReset(2);
	contextp->commandArgs(argc, argv);

	Verilated::traceEverOn(true);
	VerilatedVcdC* tfp = new VerilatedVcdC;
	top->trace(tfp, 0);
	tfp->open("obj_dir/simx.vcd");
	
	// set Vtop's input signals
	time_t t;
	srand((unsigned) time(&t));
	int f = 0;
	while (!contextp->gotFinish() && contextp->time() < 100 ) {
		nvboard_update();
		contextp->timeInc(1);
		top->a = rand() & 1;
		top->b = rand() & 1;
		top->eval();
		tfp->dump(contextp->time());	
		printf("[%" PRId64 "]: a = %d, b = %d, f = %d\n", contextp->time(), top->a, top->b, top->f); 
		assert(top->f == (top->a ^ top->b));	
	}
	top->final();
	tfp->close();
	nvboard_quit();
	
	return 0;
}
