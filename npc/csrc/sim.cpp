<<<<<<< HEAD
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

// from arch.cpp
extern const char *regs[];

// npc regs
extern uint32_t npc_regs[16];

uint32_t update_reg_no();
uint32_t update_reg_data();
void get_npc_regs() {
	uint32_t* ptr = NULL;
	ptr = (top->rootp->top__DOT__idu__DOT__regfile__DOT__rf).data();
	for(int i = 0; i < 16; i++){
		npc_regs[i] = ptr[i];
	}
	// 这是下周期要更新的 regfile 数据，本周期要拿来difftest_step
	uint32_t no = update_reg_no();
	if ( no != 0) {
		npc_regs[no] = update_reg_data();
	}
=======
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

// execute one clock cycle
static void sim_one_cycle() {
	for(int i = 0; i < 2; i++) {
		top->clock ^= 1;
		step_and_dump_wave();

#ifdef CONNECT_NVBOARD
    nvboard_update();
#endif
	}
}

// execute one inst, until EXU ready_go is 1
// return 0 --- no sepcial
// return 1 ---> function call 
// return 2 ---> function ret
// return 3 ---> ebreak_inst
int sim_once() {
	int ret = 0;
  // EXU is ready to go;
	while ( check_inst_executed_already() != true ) {

		if (check_access_fault_ifu() ) {
			printf("IFU 'pc' = '%#x' Access fault! Please check!\n", get_pc() );
			return 0;
		}
    /* check if inst is ebreak, jal, jalr ... */
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
	
  // wait one more cycle to let write to regfile
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

#ifdef WAVE_FILE
	contextp->traceEverOn(true);
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
>>>>>>> tracer-ysyx
}

void isa_reg_display() {
	get_npc_regs();
  for( int i = 0; i < 16; i++){
		printf("%s: %#x\n", regs[i], npc_regs[i]);
  }
}
<<<<<<< HEAD

=======
>>>>>>> tracer-ysyx
