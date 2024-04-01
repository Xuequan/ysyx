#ifndef __SIM_H__
#define __SIM_H__

#include <cstdint>
#include "Vtop.h"
#include "verilated_vcd_c.h"
#include "verilated.h"
#include "Vtop__Dpi.h"
#include "svdpi.h"


#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

void step_and_dump_wave();
void sim_init();
void sim_exit();
bool inst_ebreak();
uint32_t get_pc_from_top();
uint32_t get_inst_from_top();
void sim_once();
uint32_t get_clk_from_top();

#endif
