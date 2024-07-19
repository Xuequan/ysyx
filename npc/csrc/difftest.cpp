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

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

#define MROM_BASE 0x20000000
extern char *img_file;

static bool is_skip_ref = false;

extern uint32_t npc_regs[16];
void isa_reg_display();
void get_npc_regs();
uint32_t get_pc();

uint8_t* guest_to_host(paddr_t paddr);

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(word_t NO) = NULL;
void (*ref_difftest_init)(int port) = NULL;


long test_size;
void init_difftest(char *ref_so_file, long img_size, int port) {
	
  if (ref_so_file == NULL) {
		printf("no input ref_so_file\n");
		return;
	}

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
	// copy NPC flash data to NEMU flash
  ref_difftest_memcpy(FLASH_BASE, flash_guest_to_host(FLASH_BASE), test_size, DIFFTEST_TO_REF);
	get_npc_regs();
	uint32_t buf[16] = {0};
	memcpy(buf, npc_regs, 16 * sizeof(npc_regs[0]));	
  ref_difftest_regcpy(buf, DIFFTEST_TO_REF);
}

void difftest_skip_ref() {
	is_skip_ref = true;
}

/* 会在 npc-exec() 中被调用，在NEMU执行完一条指令后，就在
** difftest_step() 中让REF 执行相同的指令，然后读出REF
** 中的寄存器，并进行对比
*/
void difftest_step() {

	// 还是要让nemu执行，否则下一周期，NEMU 还是要执行该指令；
	// 因为这里，我没有将pc 复制给 nemu, 因此 nemu pc 不得更新...
	// 只是不比较 regs
  ref_difftest_exec(1);

	if (is_skip_ref) {
		is_skip_ref = false;
		// 将NPC regs 复制给 REF
		get_npc_regs();
		uint32_t buf[16] = {0};
		memcpy(buf, npc_regs, 16 * sizeof(npc_regs[0]));	
  	ref_difftest_regcpy(buf, DIFFTEST_TO_REF);

		return;
	}

	// 得到 ref 的 regs
	uint32_t ref_regs[16] = {0};
  ref_difftest_regcpy(ref_regs, DIFFTEST_TO_DUT);

	get_npc_regs();
	int error[16] = {0};
	int error_cnt = 0;
	int index = 0;
	int i = 0;
	for( ; i < 16; i++) {
		if (npc_regs[i] != ref_regs[i]) {
			error[error_cnt++] = i;
		}
	}
	if (error_cnt != 0) {
    npc_state.state = NPC_ABORT;
    npc_state.halt_pc = get_pc();
		npc_state.halt_ret = 1;

		printf("\n");
		for( int j = 0; j < error_cnt; j++){
			index = error[j];
			printf("PC = '%#x', Register '%s' in NPC is '%#x', should be '%#x'\n", 
				get_pc(), reg_name(index), npc_regs[index], ref_regs[index]);
		}

		printf("\n");
			// print REF registers
			printf("ref registers: \n");
			for(int k = 0; k < 16; k++)
				printf("%s: %#x\n", reg_name(k), ref_regs[k]);
	}
	return;
}
