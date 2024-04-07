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

#include <dlfcn.h>
#include <utility>
#include "common2.h"
#include <cstdio>

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

//extern uint32_t* npc_regs;
uint8_t* guest_to_host(paddr_t paddr);

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(word_t NO) = NULL;
void (*ref_difftest_init)(int port) = NULL;

void init_difftest(char *ref_so_file, long img_size, int port) {
	
  assert(ref_so_file != NULL);

	printf("ref_so_file : %s\n", ref_so_file);

  void *handle = NULL;
  handle = dlopen(ref_so_file, RTLD_LAZY);
	if (handle == NULL) {
		printf("cannot open so file\n");
		return;
	}
  assert(handle);

  ref_difftest_memcpy = (void (*)(paddr_t, void *, size_t, bool))dlsym(handle, "difftest_memcpy");
	if (ref_difftest_memcpy == NULL) {
		printf("get ref_difftest_memcpy error\n");
		return;
	}
  assert(ref_difftest_memcpy);

	printf("3here in init_difftest\n");
  ref_difftest_regcpy = (void (*)(void *, bool))dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

	printf("4here in init_difftest\n");
  ref_difftest_exec = (void (*)(uint64_t))dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

	printf("5here in init_difftest\n");
  ref_difftest_raise_intr = (void (*)(word_t))dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

	printf("6here in init_difftest\n");
  ref_difftest_init = (void (*)(int))dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);

  //ref_difftest_init(port);
  //ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  //ref_difftest_regcpy((void *)npc_regs, DIFFTEST_TO_REF);

	dlclose(handle);
	printf("here in init_difftest\n");
}

/*
static void checkregs(CPU_state *ref, vaddr_t pc) {
  if (!isa_difftest_checkregs(ref, pc)) {
    nemu_state.state = NEMU_ABORT;
    nemu_state.halt_pc = pc;
    isa_reg_display();
  }
}
*/

/* 会在 cpu-exec() 中被调用，在NEMU执行完一条指令后，就在
** difftest_step() 中让REF 执行相同的指令，然后读出REF
** 中的寄存器，并进行对比
*/
/*
void difftest_step(vaddr_t pc, vaddr_t npc) {
  CPU_state ref_r;

  if (skip_dut_nr_inst > 0) {
    ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
    if (ref_r.pc == npc) {
      skip_dut_nr_inst = 0;
      checkregs(&ref_r, npc);
      return;
    }
    skip_dut_nr_inst --;
    if (skip_dut_nr_inst == 0)
      panic("can not catch up with ref.pc = " FMT_WORD " at pc = " FMT_WORD, ref_r.pc, pc);
    return;
  }

  if (is_skip_ref) {
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    is_skip_ref = false;
    return;
  }

  ref_difftest_exec(1);
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);

  checkregs(&ref_r, pc);
}
*/
//#else
//void init_difftest(char *ref_so_file, long img_size, int port) { }
//#endif
