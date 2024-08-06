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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

#include "../monitor/sdb/sdb.h"

/* iringbuf */
#define IRINGBUF_LEN 15
static char iringbuf[IRINGBUF_LEN][128];
/* iindex points to the next to be written position of iringbuf[][] */
static int iindex = 0;

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();

static void print_iringbuf(void) {
	for( int i = 0; i < IRINGBUF_LEN; i++) {
<<<<<<< HEAD
		if (strlen(iringbuf[i]) != 0) {
=======
		//if (strlen(iringbuf[i]) != '\0') {
		if (iringbuf[i][0] != '\0') {
>>>>>>> tracer-ysyx
			if ( i == iindex - 1)
				printf("--> ");
			printf("%s\n", iringbuf[i]);
		}
	}
}

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
	// print inst information
	// eg: 0x80000000: 00 00 02 97 auipc	t0, 0
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
	
#endif

  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
	// check if reach breakpoint
	scan_wp_pool();
}


#ifdef CONFIG_DIFFTEST 

void vaddr2func(vaddr_t addr, bool *success, int choose, char* func_name, int len);
#define FUNC_NAME_LEN 102
static int space = 4;

/* return 1 if function call
** return 2 if function ret 
** else return 0.
*/
static int identify_inst(vaddr_t pc, word_t inst) {

  uint64_t key = 0, mask = 0, shift = 0;

  char *str_jal = "??????? ????? ????? ??? ????? 11011 11"; 
  pattern_decode(str_jal, strlen(str_jal), &key, &mask, &shift);  

	if ( (((uint64_t)inst >> shift) & mask) == key) {
    return 1;
  }
   
  char *str_jalr = "??????? ????? ????? 000 ????? 11001 11";
  pattern_decode(str_jalr, strlen(str_jal), &key, &mask, &shift);
  if ( (((uint64_t)inst >> shift) & mask) == key) {
  	uint8_t rs1 = BITS(inst, 19, 15);
  	uint8_t rd = BITS(inst, 11, 7);  
    // "jalr performs a procedure return by selecting
    // the ra as the source register and the zero register(x0)
    // as the destination register."
    if (rs1 == 1 && rd == 0){        // ret
      return 2; 
    }else if (rs1 == 1 && rd == 1){// call far-away sunroutine
      return 2; 
    }else if (rs1 == 6 && rd == 0) {// tail call far-away subroutine   
			return 2;
    }else {  					 // jr rs; jalr rs;   
			return 1;
		}
  }
  return 0;
}      
#endif

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);

// 若是将NEMU 作为NPC 的 ref, 那么就关掉 ftrace, 因为没有 ELF 文件输入
#ifdef CONFIG_DIFFTEST 
	/* ftrace start */
	bool success1 = false;
	bool success2 = false;
	char func_name[FUNC_NAME_LEN];
	int len = FUNC_NAME_LEN;

	// fliter "putch" function
	//char *_out_char_name = "putch";

	int ident = identify_inst(s->pc, s->isa.inst.val);
	if (1 == ident){ // maybe a function call, should double check 
		vaddr2func(s->dnpc, &success1, 1, func_name, len); 
		if (success1){ // double check, if next_pc is a function, then a function call
			space++;
			//if (strcmp(_out_char_name, func_name) != 0) {
				log_write("%#x:%*s [%s@%#x]\n", s->pc, space, "call", func_name, s->dnpc);
				/*
  			if (g_print_step) { //单步执行
					log_write("%#x:%*s [%s@%#x]\n", s->pc, space, "call", func_name, s->dnpc);
				}else{
					printf("%#x:%*s [%s@%#x]\n", s->pc, space, "call", func_name, s->dnpc);
				}
				*/
			//}
		}
	}else if(2 == ident){ // ret
			// call vaddr2func just for function name only
		vaddr2func(s->pc, &success2, 0, func_name, len); 
		if (success2){
			space--;
			//if (strcmp(_out_char_name, func_name) != 0) {
				log_write("%#x:%*s [%s]\n", s->pc, space, "ret ", func_name);
				/*
  			if (g_print_step) { // 单步执行
					log_write("%#x:%*s [%s]\n", s->pc, space, "ret ", func_name);
				}else{
					printf("%#x:%*s [%s]\n", s->pc, space, "ret ", func_name);
				}
				*/
			//}
		}
		else{  // should never be here
			log_write("NEMU-Should check! '%#x': inst = '%#x' is not a function entry!\n", s->pc, s->isa.inst.val);
		}
	}
	/* ftrace end */
#endif

<<<<<<< HEAD
  cpu.pc = s->dnpc;
=======
  cpu.pc = s->dnpc;   // note: until now update cpu.pc
>>>>>>> tracer-ysyx
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
	/* add machine code to logbuf
	** eg, 00 00 02 97 
	*/
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }

  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
	
	if (iindex == IRINGBUF_LEN) 
	{	iindex = 0;	}
<<<<<<< HEAD
=======
	memset(iringbuf[iindex], '\0', 128);
>>>>>>> tracer-ysyx
	memcpy(iringbuf[iindex], s->logbuf, strlen(s->logbuf));
	iindex++; 

#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
			if (nemu_state.halt_ret != 0)
				print_iringbuf();
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            													 ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic(); 
  }
}
