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
#include <ctime>

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
uint8_t* flash_guest_to_host(paddr_t paddr) { 
	return pflash + paddr - FLASH_BASE; 
}
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

void difftest_skip_ref();

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
static word_t pflash_read(paddr_t addr, int len) {
  word_t ret = host_read(flash_guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

uint32_t get_pc();
static void out_of_bound(paddr_t addr) {
  panic("NPC: address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, get_pc());
}

void init_mem() {
	uint32_t *p = (uint32_t *)pmem;
	int i;
	for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
		p[i] = 0;
	}
	Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
	return;
}

void init_flash() {
	uint32_t *p = (uint32_t *)pflash;
	int i;
	for (i = 0; i < (int) (FLASH_SIZE / sizeof(p[0])); i ++) {
		p[i] = i;
	}
	Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", FLASH_BASE, FLASH_BASE + FLASH_SIZE);
	return;
}

uint32_t nextpc();

word_t paddr_read(paddr_t addr, int len) {
	if (likely(in_pmem(addr))) {
		word_t num = pmem_read(addr, len); 
		if (nextpc() != addr) { // 过滤掉读指令
			log_write("		NPC: Read mem at address = %#x, data = %#x, now PC = %#x\n", addr, num, get_pc()); 
		}
		return num;
	}

	if (addr == (uint32_t)(RTC_ADDR) || 
			addr == (uint32_t)(RTC_ADDR + 4) ) {
		printf("should not be here\n");
		uint64_t timer = get_time();
		if (addr == (uint32_t)(RTC_ADDR) ) {	
			return (word_t)timer;
		}else{
			return (word_t)(timer >> 32);
		}
	}
	
	// read from flash
	if (addr >= FLASH_BASE && addr <= FLASH_BASE + FLASH_SIZE) {
		word_t num = pflash_read(addr, len); 
		//printf("here in flash read, address = %#x, pc = %#x\n", addr, get_pc());
		return num;		
	}
	// just for mrom
	if (addr >= 0x20000000 && addr <= 0x20000fff) {
		int idx = (addr - 0x20000000)/4;
		// 因为实际上load_img() 是将其读到 pmem 处存着的
		return *((uint32_t *)pmem + idx);		
	}
	// for uart
	if (addr >= 0x10000000 && addr <= 0x10000fff) {
		printf("read from uart '%#x'\n", addr);
		return 0;
	}

	printf("read out of bound--");	
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
		log_write("		NPC: Write mem at address = %#x, data = %#x, now PC = %#x\n", addr, data, get_pc()); 
		pmem_write(addr, len, data); 
		return; 
	}

	if (addr == (uint32_t)(SERIAL_PORT) ) {
		// 若是外设，则让 ref 跳过
		//difftest_skip_ref();

		if (len == 1) { 
			putchar(data);
			return;
		} else if(len == 2) {
			putchar((char)(data & 0xff) );
			return;
		} else if (len == 4) {
			putchar((char)data);
			return;
		} else {
			printf("paddr_write(): len = %d is wrong\n", len);
			return;
		}
	}

	if (addr == (uint32_t)(RTC_ADDR) || 
			addr == (uint32_t)(RTC_ADDR + 4) ) {
		difftest_skip_ref();
		//pmem_write(addr, len, data); 
		//关于时钟，__am_time_init() 要写入内存，NPC就直接跳过了，不写了
		return;
	}
	printf("write out of bound--");	
  out_of_bound(addr);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}
