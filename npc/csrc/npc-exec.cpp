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
#include "sim.h"

/*iringbuf */
#define IRINGBUF_LEN 15
static char iringbuf[IRINGBUF_LEN][128];
static int iindex;

static void print_iringbuf(void) {
	for(int i = 0; i < IRINGBUF_LEN; i++){
		if (strlen(iringbuf[i]) != 0) {
			if (i == iindex - 1) 
				printf("--> ");
			printf("%s\n", iringbuf[i]);
		}
	}
}
void disassemble(char *str, int size, uint64_t pc, uint8_t* code, int nbyte);
static char logbuf[128];

void get_assemble() {
	char *p = logbuf;
	uint32_t pc 				 = get_pc_from_top();
	uint32_t instruction = get_inst_from_top();
	uint8_t* inst = (uint8_t *)&instruction;
	for(int k = 3; k >= 0; k--) {
		p += snprintf(p, 4, " %02x", inst[k]);
	}
	p += 1;
	disassemble(p, logbuf + sizeof(logbuf) - p, pc, inst, 4);
	printf("%#08x:%s %s\n",pc, logbuf, p);
}

bool inst_is_ebreak();
bool inst_is_jal();
bool inst_is_jalr();
uint32_t rs1();
/* return 1 if reach ebreak instruction else 0 */
int exec_once() {
  int ret = 0;
  uint32_t pc; 
  uint32_t inst;
  for(int i = 0; i < 2; i++) {
		sim_once();
    if (get_clk_from_top() == 1) {
      get_assemble();
			if (iindex == IRINGBUF_LEN) iindex = 0;
			memcpy(iringbuf[iindex++], logbuf, strlen(logbuf));
      //printf("pc = %#08x, inst = %08x\n", top->pc, top->inst);
			printf("rs1 is %d\n", rs1());
      if (inst_is_ebreak() ) { 
        ret = 1;
        break;
      }   
    }   
  }// end for
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
    	printf("\nReach ebreak instruction, stop sim.\n\n");
			npc_state.state = NPC_END;
			npc_state.halt_pc = get_pc_from_top();
			npc_state.halt_ret = 0;
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

      if (npc_state.halt_ret != 0)
        print_iringbuf();
      // fall through
    	case NPC_QUIT: ;//statistic(); 
  	}
}
