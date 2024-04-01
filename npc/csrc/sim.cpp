#include "sim.h"

static Vtop* top;
static VerilatedContext* contextp;
static VerilatedVcdC* tfp;

//static svScope scope;

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

	/* DPI-C 接口 */
	const svScope scope = svGetScopeFromName("TOP.top");
	//scope = svGetScopeFromName("TOP.top->ysyx_23060208_IDU");
	assert(scope);
	svSetScope(scope);

	int i = -1;
	while ( i < 5) {
		i++;
		top->clk ^= 1;
		top->rst = 1;
		step_and_dump_wave();
	}
	top->rst = 0;
}

void sim_exit() {
	tfp->close();
	delete tfp;
	delete top;
	delete contextp;
}

bool inst_ebreak() {
	svBit a; 
	top->check_if_ebreak(&a);
	if ( a == 1) return true;
	else 				 return false;
} 

uint32_t get_pc_from_top(){
	return top->pc;
}
uint32_t get_inst_from_top(){
	return top->inst;
}
uint32_t get_clk_from_top(){
	return top->clk;
}

void sim_once() {
	top->clk ^= 1;
	step_and_dump_wave();
}
