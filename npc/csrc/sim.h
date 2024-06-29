#ifndef __SIM_H__
#define __SIM_H__

#include <cstdint>
extern void sim_init();
extern void sim_exit();
extern int sim_once();
extern uint32_t get_clk_from_top();

extern void isa_reg_display();

extern void get_npc_regs();

#endif
