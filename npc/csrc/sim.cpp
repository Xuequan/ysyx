#include <cstdint>
#include "VysyxSoCFull.h"
#include "verilated_vcd_c.h"
#include "VysyxSoCFull__Dpi.h"
#include "VysyxSoCFull___024root.h"
#include "dpi-c.h"
#include "ctrl.h"

#ifdef CONNECT_NVBOARD
#include "nvboard.h"
void nvboard_bind_all_pins(VysyxSoCFull* top);
void nvboard_quit();
#endif

static VysyxSoCFull* top;
static VerilatedContext* contextp;
static VerilatedVcdC* tfp;


static void step_and_dump_wave() {
	top->eval();
	// generate wave file
#ifdef WAVE_FILE
	contextp->timeInc(1);
	tfp->dump(contextp->time());
#endif
}

// execute one cycle
static void sim_one_cycle() {
	for(int i = 0; i < 2; i++) {
		top->clock ^= 1;
		step_and_dump_wave();

#ifdef CONNECT_NVBOARD
    nvboard_update();
#endif
	}
}

// execute one inst
// return 0 --- no sepcial
// return 1 ---> function call 
// return 2 ---> function ret
// return 3 ---> ebreak_inst
int sim_once() {
	int ret = 0;
	while ( check_exu_ready_go() != true ) {

		if (check_access_fault_ifu() ) {
			printf("IFU 'pc' = '%#x' Access fault! Please check!\n", get_pc() );
			return 0;
		}

		if (inst_is_ebreak()){
			ret = 3;
		}else if (inst_is_jal()){
			ret = 1;
		}else if (inst_is_jalr()){
			if (rs1() == 1 && rd() == 0) 
				ret = 2;
			else if(rs1() == 1 && rd() == 1)
				ret = 2;
			else if(rs1() == 6 && rd() == 0)
				ret = 2;
			else 
				ret = 1;
		}
		sim_one_cycle();
	} // end-while

	if (check_access_fault_exu() ) {
		printf("EXU 'pc' = '%#x' Access fault! Please check!\n", get_pc() );
		return 0;
	}
		
	sim_one_cycle();
	return ret; 
}

void sim_init() {
 	contextp = new VerilatedContext;
	tfp = new VerilatedVcdC;
	top = new VysyxSoCFull;

#ifdef CONNECT_NVBOARD
  nvboard_bind_all_pins(top);
  nvboard_init();
#endif

	contextp->traceEverOn(true);

#ifdef WAVE_FILE
	top->trace(tfp, 0);
  tfp->open("dump.vcd");
#endif

	// initial reset
	int i = -1;
	while ( i < 29) {
		i++;
		top->clock ^= 1;
		top->reset = 1;
		step_and_dump_wave();
	}
	top->reset = 0;
	top->clock ^= 1;
	step_and_dump_wave();
}

void sim_exit() {
#ifdef WAVE_FILE
	tfp->close();
	delete tfp;
#endif
	delete top;
	delete contextp;

#ifdef CONNECT_NVBOARD
  nvboard_quit();
#endif
}

uint32_t get_clock_from_top(){
	return top->clock;
}
// from arch.cpp
extern const char *regs[];
extern uint32_t npc_regs[16];
extern uint32_t update_reg_no();
extern uint32_t update_reg_data();
void get_npc_regs() {
	uint32_t* ptr = NULL;
	ptr = (top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__idu__DOT__regfile__DOT__rf).data();
	for(int i = 0; i < 16; i++){
		npc_regs[i] = ptr[i];
	}
}

void isa_reg_display() {
	get_npc_regs();
  for( int i = 0; i < 16; i++){
		printf("%s: %#x\n", regs[i], npc_regs[i]);
  }
}
