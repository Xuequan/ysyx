#include "sim.h"

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

extern const char *regs[];
//uint32_t* npc_regs = (top->rootp->top__DOT__idu__DOT__regfile__DOT__rf).data();

uint32_t npc_regs[16];
void get_npc_regs() {
	uint32_t* ptr = NULL;
	ptr = (top->rootp->top__DOT__idu__DOT__regfile__DOT__rf).data();
	for(int i = 0; i < 16; i++){
		npc_regs[i] = ptr[i];
	}
}

void isa_reg_display() {
	/*
  VlUnpacked<IData, 32> tmp = top->rootp->top__DOT__idu__DOT__regfile__DOT__rf;
  for( int i = 0; i < 16; i++){
		//printf("%s: %#x\n", regs[i], tmp.m_storage[i]);
		printf("%s: %#x\n", regs[i], *(uint32_t *)(tmp.data() + i));
		
  }
	*/
	get_npc_regs();
  for( int i = 0; i < 16; i++){
		printf("%s: %#x\n", regs[i], npc_regs[i]);
  }
}

