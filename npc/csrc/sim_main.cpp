#include <memory>
#include <verilated.h>
#include <Vtop.h>

#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include "verilated_vcd_c.h"

int main(int argc, char** argv) {
	if (false && argc && argv) {}

	const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};

	const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};

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
	
	return 0;
}
