#include <cstdlib>
//#include <cstdint>
#include "sim.h"
#include "Vtop.h"
#include "verilated_vcd_c.h"
#include "verilated.h"
#include "Vtop__Dpi.h"
#include "svdpi.h"

static Vtop* top;
static VerilatedContext* contextp;
static VerilatedVcdC* tfp;

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
	int i = 2;
	while (i--) {	
		top->clk ^= 1;
		printf("top->clk = %d\n", top->clk);
		top->rst = 1;
	}
	tfp->close();
	delete tfp;
	delete top;
	delete contextp;
}

bool inst_ebreak() {
	svBit a; 
	top->check_ebreak(&a);
	if ( a == 1) return true;
	else 				 return false;
} 

uint32_t get_pc_from_top(){
	return top->pc;
}
uint32_t get_inst_from_top(){
	return top->inst;
}
/* return 1 if reach ebreak instruction else 0 */
int exec_once() {
	int ret = 0;
	uint32_t pc;
	uint32_t inst;
	for(int i = 0; i < 2; i++) {
		top->clk ^= 1;
		step_and_dump_wave();
		if (top->clk == 1) {
			printf("pc = %#08x, inst = %#08x\n", top->pc, top->inst);
		}
		if (inst_ebreak() ) {
			ret = 1;
			break;
		}	
	}
	return ret;
}
