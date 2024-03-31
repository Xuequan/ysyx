/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
#include "common2.h"
#include "arch.h"
#include <cstdio>

#include "Vtop.h"
#include "Vtop__Dpi.h"
#include "svdpi.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
	/*
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
	*/
};

#define NR_REGS ARRLEN(regs)
void isa_reg_display() {
	int i = 0;
	for (; i < NR_REGS; i++) {
		printf("%-5s %#x\n", regs[i], gpr(i));  
	}
}

word_t isa_reg_str2val(const char *s, bool *success) {
	int i = 0;
	for( ; i < NR_REGS; i++) {
		if (strcmp(s, regs[i]) == 0 ) 
			break;
	}	
	if (i == NR_REGS){
		*success = false;
		return 0;
	} else {
		*success = true;
	}
	word_t reg_val = gpr(i);	
  return reg_val;
}

void assert_fail_msg() {
	isa_reg_display();
	//statistic();
}

//static char logbuf[128];
extern Vtop* top;
extern svBit a;
extern void step_and_dump_wave();
// 执行一个cycle
// return 1 if program ended
int exec_once() {	
	int ret = 0;
	for(int i = 0; i < 2; i++) {
		top->clk ^= 1;
  	step_and_dump_wave();
		if ( top->clk == 1) {
  		printf("pc = 08%08x, inst = 08%08x, clk->rst = %d\n",
				top->pc,  top->inst, top->rst);
		}

  	top->check_ebreak(&a);
  	if (a == 1) {
    	printf("\nReach ebreak instruction, stop sim.\n\n");
			npc_state.state = NPC_END;
			npc_state.halt_pc = top->pc;
			npc_state.halt_ret = 0;
			ret = 1;
  	}
	} // end for
	return ret;
}

void execute(uint64_t n) {
	//g_print_step = (n < MAX_INST_TO_PRINT);
	switch (npc_state.state) {
		case NPC_END: case NPC_ABORT:
			printf("Program execution has ended. To restart the program, please exit and restart\n");
			return;
		default: 
				npc_state.state = NPC_RUNNING;
	}

	for( ; n > 0; n--) {
		if (1 == exec_once()) {
			break;
		}
	}

	switch (npc_state.state) {
		case NPC_RUNNING: 
			npc_state.state = NPC_STOP; break;

	    case NPC_END: case NPC_ABORT:
      Log("npc: %s at pc = " FMT_WORD,
          (npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
                                       ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          npc_state.halt_pc);
			/*
      if (npc_state.halt_ret != 0)
        print_iringbuf();
			*/
      // fall through
    	case NPC_QUIT: ;//statistic(); 
  	}
}
