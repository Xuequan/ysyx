#ifndef __SIM_H__
#define __SIM_H__

#include <cstdint>
<<<<<<< HEAD
#include "Vtop.h"
#include "verilated_vcd_c.h"
#include "Vtop__Dpi.h"
#include "Vtop___024root.h"

void step_and_dump_wave();
void sim_init();
void sim_exit();
uint32_t get_pc_from_top();
uint32_t get_inst_from_top();
void sim_once();
uint32_t get_clk_from_top();

void isa_reg_display();

void get_npc_regs();
=======
extern void sim_init();
extern void sim_exit();
extern int sim_once();
extern uint32_t get_clk_from_top();

extern void isa_reg_display();

extern void get_npc_regs();
>>>>>>> tracer-ysyx

#endif
