#ifndef __SIM_H__
#define __SIM_H__

#include <cstdint>
//#include "verilated.h"
//#include "verilated_vcd_c.h"
//#include "Vtop.h"

/*
extern VerilatedContext* contextp = new VerilatedContext;
extern VerilatedVcdC* tfp = new VerilatedVcdC;
extern Vtop* top = new Vtop;
extern svBit a; 
extern VerilatedContext* contextp;
extern VerilatedVcdC* tfp;
extern Vtop* top;
*/

void step_and_dump_wave();
void sim_init();
void sim_exit();
bool inst_ebreak();
int exec_once();
uint32_t get_pc_from_top();
uint32_t get_inst_from_top();

#endif
