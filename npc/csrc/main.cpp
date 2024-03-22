#include "Vtop.h"
#include <stdlib.h>
#include "verilated_vcd_c.h"
#include "verilated.h"

#include <stdio.h>
/*
#include "Vtop__Dpi.h"
#include "svdpi.h"
*/

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

int main() {
	sim_init();
	/*
	const svScope scope = svGetScopeFromName("TOP.top");
	assert(scope);
	svSetScope(scope);
	svBit a; 
	*/

	int i = -1;
	while (1) {
		i++;
		top->clk = i % 2;
		if (i < 5) { 
			top->rst = 1;	
			step_and_dump_wave();
			continue;
		} else { 
			top->rst = 0;	
		}
		
		//bool success = 0;

		/*
		if (top->clk) { 
			top->check_ebreak(&a);
			if (a == 1) {
				printf("Reach ebreak instruction, stop sim.\n");
				step_and_dump_wave();
				break;
			}
		}
		*/
		step_and_dump_wave();
		if (top->inst_ebreak){
			printf("hit ebreak inst\n");
			return 0;
		}
	} // end while(1)
	sim_exit();
	return 0;
}
