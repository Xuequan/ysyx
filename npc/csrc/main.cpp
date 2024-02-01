#include "Vtop.h"
#include <stdlib.h>
#include "nvboard.h"
#include "verilated_vcd_c.h"
#include "verilated.h"

// static TOP_NAME dut;

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

static Vtop* top;

void step_and_dump_wave() {
	top->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}
void sim_init() {
	contextp = new VerilatedContext;
	tfp = new VerilatedVcdC;
	top = new Vtop;
	contextp->traceEverOn(true);
	top->trace(tfp, 0);
	tfp->open("dump.vcd");
}
void sim_exit() {
	tfp->close();
}
void nvboard_bind_all_pins(Vtop *top) {
	nvboard_bind_pin(&top->a, false, true, 1, LD0);
	nvboard_bind_pin(&top->b, false, true, 1, LD0);
	nvboard_bind_pin(&top->f, false, false, 1, LD0);
}

int main() {
	sim_init();

	nvboard_bind_all_pins(top);
	nvboard_init();
	
	while (1) {
		top->a = rand() & 1;
		top->b = rand() & 1;
		step_and_dump_wave();

		nvboard_update();
	}
	//nvboard_quit();
	
	sim_exit();
	return 0;
}
