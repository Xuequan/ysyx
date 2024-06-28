#include <cstdint>
#include "Vtop.h"
#include "verilated_vcd_c.h"
#include "Vtop__Dpi.h"
#include "Vtop___024root.h"
#include "dpi-c.h"

static Vtop* top;
static VerilatedContext* contextp;
static VerilatedVcdC* tfp;

static void step_and_dump_wave() {
	top->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}

// execute one cycle
static void sim_one_cycle() {
	for(int i = 0; i < 2; i++) {
		top->clk ^= 1;
		step_and_dump_wave();
	}
}

// execute one inst
// return 0 --- no sepcial
// return 1 ---> function call 
// return 2 ---> function ret
// return 3 ---> ebreak_inst
int sim_once() {
	while ( check_exu_ready_go() != true ) {
		if (inst_is_ebreak()){
			return 3;
		}else if (inst_is_jal()){
			return 1;
		}else if (inst_is_jalr()){
			if (rs1() == 1 && rd() == 0) 
				return 2;
			else if(rs1() == 1 && rd() == 1)
				return 2;
			else if(rs1() == 6 && rd() == 0)
				return 2;
			else 
				return 1;
		}
		sim_one_cycle();
	} // end-while
	
	sim_one_cycle(); // need this, let write regfile
	return 0; 
}

void sim_init() {
 	contextp = new VerilatedContext;
	tfp = new VerilatedVcdC;
	top = new Vtop;

	contextp->traceEverOn(true);
	top->trace(tfp, 0);
	tfp->open("dump.vcd");

	// initial rst
	int i = -1;
	while ( i < 5) {
		i++;
		top->clk ^= 1;
		top->rst = 1;
		step_and_dump_wave();
	}
	top->rst = 0;
	/*
	while( check_ifu_ready_go() != true) {
		sim_one_cycle();
	}
	*/
}

void sim_exit() {
	tfp->close();
	delete tfp;
	delete top;
	delete contextp;
}

uint32_t get_clk_from_top(){
	return top->clk;
}

// from arch.cpp
extern const char *regs[];

// npc regs
extern uint32_t npc_regs[16];

//uint32_t update_reg_no();
//uint32_t update_reg_data();
void get_npc_regs() {
	uint32_t* ptr = NULL;
	ptr = (top->rootp->top__DOT__idu__DOT__regfile__DOT__rf).data();
	for(int i = 0; i < 16; i++){
		npc_regs[i] = ptr[i];
	}
}
	/*
	// 这是下周期要更新的 regfile 数据，本周期要拿来difftest_step
	// 对于单周期NPC，寄存器的更新要在下一周期
	uint32_t no = update_reg_no();
	if ( no != 0) {
		npc_regs[no] = update_reg_data();
	}
	*/

void isa_reg_display() {
	get_npc_regs();
  for( int i = 0; i < 16; i++){
		printf("%s: %#x\n", regs[i], npc_regs[i]);
  }
}
