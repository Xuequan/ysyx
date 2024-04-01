#ifndef __SIM_H__
#define __SIM_H__

#include <cstdint>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

void step_and_dump_wave();
void sim_init();
void sim_exit();
bool inst_ebreak();
int exec_once();
uint32_t get_pc_from_top();
uint32_t get_inst_from_top();

#endif
