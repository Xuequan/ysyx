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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

/* 对比一致则返回 true */ 
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
	int i = 0;
	int error[32] = {0};
	int cnt = 0;
	for( ; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
		if (gpr(i) != ref_r->gpr[i]) {
			printf("Registers of NEMU and REF are different, PC = %#x\n", pc);
			error[cnt++] = i;
		}
	}
	if (cnt != 0) {
		for(i = 0; i < cnt; i++) {
			printf("Different register: '%s', '%#x' in NEMU, '%#x' in REF\n", reg_name(error[i]), gpr(error[i]), ref_r->gpr[error[i]]);
		}
		return false;
	}
  return true;
	
}

void isa_difftest_attach() {
}
