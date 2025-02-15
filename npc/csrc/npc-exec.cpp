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
#include "dpi-c.h"
<<<<<<< HEAD
=======
#include <clocale>
#include "ctrl.h"
>>>>>>> tracer-ysyx

#define MAX_INST_TO_PRINT 10
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
uint64_t g_nr_guest_inst = 0;

<<<<<<< HEAD
/* return 1 if function call
** return 2 if function ret
** else return 0
*/
static int identify_inst() {
	if (inst_is_jal() ){
		return 1;
	} else if (inst_is_jalr()){
		if (rs1() == 1 && rd() == 0)  // ret
			return 2;
		else if (rs1() == 1 && rd() == 1)
			return 2;
		else if (rs1() == 6 && rd() == 0)
			return 2;
		else 
			return 1;
	} else{
		return 0;
	}
}

=======
void difftest_skip_ref();	
>>>>>>> tracer-ysyx
static int space = 4;
char *vaddr2func(vaddr_t addr, bool *success, int choose, char* func_name, int len);

#define FUNC_NAME_LEN 102
<<<<<<< HEAD
static void ftrace() {
=======
static void ftrace(int ident) {
>>>>>>> tracer-ysyx
  bool success1 = false;
  bool success2 = false;
	char func_name[FUNC_NAME_LEN];
	int len = FUNC_NAME_LEN;

<<<<<<< HEAD
  int ident = identify_inst();
=======
  //int ident = identify_inst();
>>>>>>> tracer-ysyx
  if (1 == ident){ // maybe a function call, should double check
    vaddr2func(nextpc(), &success1, 1, func_name, len); 
    if (success1){ // double check, if next_pc is a function, then a function call
      space++;
<<<<<<< HEAD
      log_write("%#x:%*s [%s@%#x]\n", get_pc_from_top(), space, "call", func_name, nextpc());  
    }
  }else if(2 == ident){ // ret
      // call vaddr2func just for function name only
    vaddr2func(get_pc_from_top(), &success2, 0, func_name, len); 
    if (success2){
      space--;
      log_write("%#x:%*s [%s]\n", get_pc_from_top(), space, "ret ", func_name);
    }else{  // should never be here
      log_write("NPC--Should check! %s pc = '%#x': inst = '%#x' is not a function entry!\n", func_name, get_pc_from_top(), get_inst_from_top());
=======
#ifdef LOG_WRITE_ENABLE
      log_write("%#x:%*s [%s@%#x]\n", get_pc(), space, "call", func_name, nextpc());  
#endif
    }
  }else if(2 == ident){ // ret
      // call vaddr2func just for function name only
    vaddr2func(get_pc(), &success2, 0, func_name, len); 
    if (success2){
      space--;
#ifdef LOG_WRITE_ENABLE
      log_write("%#x:%*s [%s]\n", get_pc(), space, "ret ", func_name);
#endif
    }else{  // should never be here
      log_write("NPC--Should check! %s pc = '%#x': inst = '%#x' is not a function entry!\n", func_name, get_pc(), get_inst());
>>>>>>> tracer-ysyx
    }     
  }
}

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
<<<<<<< HEAD
static char logbuf[128];

void get_assemble() {
	char *p = logbuf;
	uint32_t pc 				 = get_pc_from_top();
	uint32_t instruction = get_inst_from_top();
	uint8_t* inst = (uint8_t *)&instruction;
=======

static char logbuf[128];

void get_assemble_code() {
	char *p = logbuf;
	uint32_t pc 				 = get_pc();
	uint32_t instruction = get_inst();
	uint8_t* inst = (uint8_t *)&instruction;

>>>>>>> tracer-ysyx
	p += snprintf(p, sizeof(logbuf), FMT_WORD ":", pc);
	for(int k = 3; k >= 0; k--) {
		p += snprintf(p, 4, " %02x", inst[k]);
	}
	memset(p, ' ', 1);
	p += 1;
	disassemble(p, logbuf + sizeof(logbuf) - p, pc, inst, 4);
<<<<<<< HEAD
	//printf("%#08x:%s %s\n",pc, logbuf, p);
=======
	/* printf each executed instruction in NPC */
	if (g_print_step){
		printf("%s\n",logbuf);
	}
>>>>>>> tracer-ysyx
}

void scan_wp_pool();
void difftest_step();

static void trace_and_difftest(){
<<<<<<< HEAD
	log_write("%s\n", logbuf);
	if (g_print_step){
		printf("%s\n",logbuf);
	}
	difftest_step();

	scan_wp_pool();
}

bool inst_is_ebreak();
bool inst_is_jal();
bool inst_is_jalr();

void exec_once() {
  for(int i = 0; i < 2; i++) {
		sim_once();
    if (get_clk_from_top() == 1) {
      get_assemble();

			if (iindex == IRINGBUF_LEN) 
				iindex = 0;
			memset(iringbuf[iindex], 0, sizeof(iringbuf[iindex]));
			memcpy(iringbuf[iindex++], logbuf, strlen(logbuf));

			ftrace();
    }   
  }// end for
}


=======
#ifdef LOG_WRITE_ENABLE
#ifdef WRITE_EVERY_INST
	log_write("%s\n", logbuf);
#endif
#endif

	if (check_clint_read() || check_uart_write() || check_uart_read() 
		|| check_spi_master_read() || check_spi_master_write()  
    || check_gpio() || check_ps2()  ) {

		difftest_skip_ref();	
  }

	difftest_step();
}

void exec_once() {
	int sim_ret = sim_once();

	get_assemble_code();

	if (iindex == IRINGBUF_LEN) 
		iindex = 0;
	memset(iringbuf[iindex], 0, sizeof(iringbuf[iindex]));
	memcpy(iringbuf[iindex++], logbuf, strlen(logbuf));

	if (sim_ret == 3) { 
#ifdef PRINT_FLASH_MEM
    void print_flash();
    void print_mem();
		printf("after reach ebreak, to see if flash, mem change\n");
		print_flash();
		print_mem();
#endif

		printf("\nReach ebreak instruction, stop sim.\n\n");
		npc_state.state = NPC_END;
		npc_state.halt_pc = get_pc();
		npc_state.halt_ret = 0;
		return;
	}

	if (sim_ret == 1 || sim_ret == 2) 
		ftrace(sim_ret);
}

>>>>>>> tracer-ysyx
void execute(uint64_t n) {
	for( ; n > 0; n--) {
		g_nr_guest_inst ++;
		exec_once();
<<<<<<< HEAD
		//trace_and_difftest();
		if (npc_state.state != NPC_RUNNING) 
			return;
    if (inst_is_ebreak() ) { 
    	printf("\nReach ebreak instruction, stop sim.\n\n");
			npc_state.state = NPC_END;
			npc_state.halt_pc = get_pc_from_top();
			npc_state.halt_ret = 0;
			return;
		}
=======
#ifdef DIFFTEST
	  trace_and_difftest();
#endif
		// breakpoint 
		scan_wp_pool();
		if (npc_state.state != NPC_RUNNING) 
			return;
>>>>>>> tracer-ysyx
	}
}

void statistic() {
  //IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
  setlocale(LC_NUMERIC, "");
#define NUMBERIC_FMT MUXDEF(false, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void npc_exec(uint64_t n) {
	g_print_step = (n < MAX_INST_TO_PRINT);
	switch (npc_state.state) {
		case NPC_END: case NPC_ABORT:
			printf("Program execution has ended. To restart the program, please exit and restart\n");
			return;
		default: 
<<<<<<< HEAD
				npc_state.state = NPC_RUNNING;
=======
			npc_state.state = NPC_RUNNING;
>>>>>>> tracer-ysyx
	}
	
	uint64_t timer_start = get_time();

	execute(n);

	uint64_t timer_end = get_time();
	g_timer += timer_end - timer_start;

	switch (npc_state.state) {
		case NPC_RUNNING: 
			npc_state.state = NPC_STOP; break;

	    case NPC_END: case NPC_ABORT:
			{
      if (npc_state.halt_ret != 0)
        print_iringbuf();
      Log("npc: %s at pc = " FMT_WORD,
          (npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
                                       ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          npc_state.halt_pc);
			 }
      // fall through
    	case NPC_QUIT: statistic(); 
  	}
}
