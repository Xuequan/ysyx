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
#include "arch.h"
#include "sim.h"

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

extern uint32_t npc_regs[16];
uint8_t* guest_to_host(paddr_t paddr);

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(word_t NO) = NULL;
void (*ref_difftest_init)(int port) = NULL;

void init_difftest(char *ref_so_file, long img_size, int port) {
	
  assert(ref_so_file != NULL);

  void *handle = NULL;
  handle = dlopen(ref_so_file, RTLD_LAZY);
  assert(handle);

  ref_difftest_memcpy = (void (*)(paddr_t, void *, size_t, bool))dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = (void (*)(void *, bool))dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = (void (*)(uint64_t))dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = (void (*)(word_t))dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  ref_difftest_init = (void (*)(int))dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);

  ref_difftest_init(port);
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
	printf("after difftest_memcpy\n");

	get_npc_regs();
	printf("after get_npc_reg()\n");
	uint32_t buf[16] = {0};
	memcpy(buf, npc_regs, 16 * sizeof(npc_regs[0]));	
	printf("after memcpy\n");
  ref_difftest_regcpy(buf, DIFFTEST_TO_REF);

	printf("after difftest_regcpy\n");
	//dlclose(handle);
}

/* 会在 cpu-exec() 中被调用，在NEMU执行完一条指令后，就在
** difftest_step() 中让REF 执行相同的指令，然后读出REF
** 中的寄存器，并进行对比
*/
void difftest_step() {

  ref_difftest_exec(1);
	printf("after ref_difftest_exec(1)\n");

	// 得到 ref 的 regs
	uint32_t ref_regs[16] = {0};
  ref_difftest_regcpy(ref_regs, DIFFTEST_TO_DUT);
	printf("after ref_difftest_regcpy\n");

	get_npc_regs();
	int i = 0;
	for( ; i < 16; i++) {
		if (npc_regs[i] != ref_regs[i]) {
    	npc_state.state = NPC_ABORT;
    	npc_state.halt_pc = get_pc_from_top();
    	isa_reg_display();
		}	
	}
}
