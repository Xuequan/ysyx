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

#define SRAM_SIZE (0x0fffffff - 0x0f000000)
#define SRAM_BASE	0x0f000000
#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *psram = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t psram[SRAM_SIZE] PG_ALIGN = {};
#endif

void init_sram() {
  uint32_t *p = (uint32_t *)psram;
  int i;
  for (i = 0; i < (int) (SRAM_SIZE / sizeof(p[0])); i ++) {
    p[i] = rand();
  }
  Log("MROM area [0x20000000, 0x20000fff]");
}

uint8_t* sram_guest_to_host(paddr_t paddr) { 
	return psram + paddr - SRAM_BASE; 
}
paddr_t host_to_guest_sram(uint8_t *haddr) { 
	return haddr - psram + SRAM_BASE; 
}
word_t psram_read(paddr_t addr, int len) {
  word_t ret = host_read(sram_guest_to_host(addr), len);
  return ret;
}

void psram_write(paddr_t addr, int len, word_t data) {
  host_write(sram_guest_to_host(addr), len, data);
}

