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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmrom = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmrom[MROM_SIZE] PG_ALIGN = {};
#endif

void init_mrom() {
  uint32_t *p = (uint32_t *)pmrom;
  int i;
  for (i = 0; i < (int) (MROM_SIZE / sizeof(p[0])); i ++) {
    //p[i] = rand();
    p[i] = 0;
  }
  Log("MROM area [0x2000_0000, 0x2000_0fff]");
}

uint8_t* mrom_guest_to_host(paddr_t paddr) { 
	return pmrom + paddr - MROM_BASE; 
}
word_t pmrom_read(paddr_t addr, int len) {
  word_t ret = host_read(mrom_guest_to_host(addr), len);
  return ret;
}
