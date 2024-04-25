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
#include "memory.h"
#include <cstdio>

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static inline word_t host_read(void *addr, int len) {
  switch (len) {
    case 1: return *(uint8_t  *)addr;
    case 2: return *(uint16_t *)addr;
    case 4: return *(uint32_t *)addr;
    default: ;
  }
	return 0;
}

static inline void host_write(void *addr, int len, word_t data) {
  switch (len) {
    case 1: *(uint8_t  *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
  }
}

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

uint32_t get_pc_from_top();
static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, get_pc_from_top());
}

void init_mem() {
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
		p[i] = 0;
  }
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

word_t paddr_read(paddr_t addr, int len) {
	if (likely(in_pmem(addr))) {
		word_t num = pmem_read(addr, len); 
		log_write("		Read to mem at address = %#x, data = %#x, now PC = %#x\n", addr, num, get_pc_from_top()); 
		return num;
	}
	out_of_bound(addr);
	return 0;
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { 
	printf("==========================1-addr = %#x\n", addr);
		log_write("		Write to mem: address = %#x, data = %#x, now PC = %#x\n", addr, data, get_pc_from_top()); 
		pmem_write(addr, len, data); 
		return; 
	}
	printf("==========================addr = %#x\n", addr);
	if (addr == (uint32_t)SERIAL_PORT) {
		if (len == 1) { 
			//return putch((char)(data & 0xf) );
			putchar((char)(data & 0xf) );
			return;
		} else if(len == 2) {
			//return putch((char)(data & 0xff) );
			putchar((char)(data & 0xff) );
			return;
		} else if (len == 4) {
			//return putch((char)data);
			putchar((char)data);
			return;
		} else {
			printf("paddr_write(): len = %d is wrong\n", len);
			return;
		}
	}
  out_of_bound(addr);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}
