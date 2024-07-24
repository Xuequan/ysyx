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

#include <isa.h>
#include <cpu/cpu.h>
#include <difftest-def.h>
#include <memory/paddr.h>
#include <string.h>

/* 哎不要忘了，NEMU 和 NPC 这两个“电脑”都运行在 host 上, 
 * 因此就用下面的 memcpy....*/
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
	if (direction == DIFFTEST_TO_REF) {
		if (addr == RESET_VECTOR) 
			memcpy(guest_to_host(RESET_VECTOR), buf, n);
		else if (addr == FLASH_BASE)
			memcpy(flash_guest_to_host(FLASH_BASE), buf, n);
	} else {
  	assert(0);
	}	
}

__EXPORT void difftest_regcpy(void *dut, bool direction) {

	if (direction == DIFFTEST_TO_REF) {
		for(int i = 0; i < RISCV_GPR_NUM; i++) {
			cpu.gpr[i] = *((word_t *)dut + i);	
		}
	} else { // copy nemu(REF) regs to dut
		memcpy(dut, cpu.gpr, 16 * sizeof(cpu.gpr[0]));
	}	
}

__EXPORT void difftest_exec(uint64_t n) {
	cpu_exec(n);
}

__EXPORT void difftest_raise_intr(word_t NO) {
  assert(0);
}

__EXPORT void difftest_init(int port) {
  void init_mem();
	void init_sram();
	void init_mrom();
	void init_flash();
	init_mrom();
	init_sram();
	init_flash();
  /* Perform ISA dependent initialization. */
  init_isa();
  init_mem();
}
