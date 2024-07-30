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

extern uint32_t get_pc();
extern uint32_t nextpc();

//=========================================================================
// address memory space translated to host memory space
// ========================================================================
// psram
uint8_t* guest_to_host(paddr_t paddr) { 
  return psram + paddr - PSRAM_BASE; 
}

// flash
uint8_t* flash_guest_to_host(paddr_t paddr) {
	return pflash + paddr - FLASH_BASE; 
}

// sdram
uint8_t* sdram_guest_to_host(paddr_t paddr) { 
	return psdram + paddr - SDRAM_BASE; 
}

// to be deleted
paddr_t host_to_guest(uint8_t *haddr) { 
  return haddr - psram + PSRAM_BASE; 
}

//=========================================================================
// host read and write 
// ========================================================================
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

//=========================================================================
// all sorts of memory space read and write 
// ========================================================================
// psram read
static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

// flash read (no write)
static word_t pflash_read(paddr_t addr, int len) {
  word_t ret = host_read(flash_guest_to_host(addr), len);
  return ret;
}

// sdram read
static word_t psdram_read(paddr_t addr, int len) {
  word_t ret = host_read(sdram_guest_to_host(addr), len);
  return ret;
}

// sdram write
static void psdram_write(paddr_t addr, int len, word_t data) {
  host_write(sdram_guest_to_host(addr), len, data);
}

// psram write
static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}


//=========================================================================
// check if addr is out of bound 
// ========================================================================
static void out_of_bound(paddr_t addr) {
  panic("NPC: address = " FMT_PADDR " is out of bound of psram [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PSRAM_BASE, PSRAM_SIZE + PSRAM_BASE, get_pc());
}


//=========================================================================
// debug helper function
// ========================================================================
void print_mem() {
	uint32_t *p = (uint32_t *)psram;
	printf("data in psram :\n");
	for(int i = 0; i < 10; i ++){
		printf("%d : %#x\n", i, p[i]);
	}
}

void print_flash() {
	uint32_t *p = (uint32_t *)pflash;
	printf("data in flash :\n");
	for(int i = 0; i < 10; i ++){
		printf("%d : %#x\n", i, p[i]);
	}
}
//=========================================================================
// initial memory space
// ========================================================================
// initial psram
void init_mem() {
	uint32_t *p = (uint32_t *)psram;
	int i;
	for (i = 0; i < (int) (PSRAM_SIZE / sizeof(p[0])); i ++) {
		p[i] = 0;
	}
	Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", 
      PSRAM_BASE, PSRAM_BASE + PSRAM_SIZE);
	return;
}

void init_flash() {
	uint32_t *p = (uint32_t *)pflash;
	int i;
	for (i = 0; i < (int) (FLASH_SIZE / sizeof(p[0])); i ++) {
			p[i] = 0;
	}
	Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", FLASH_BASE, FLASH_BASE + FLASH_SIZE);
	return;
}

void init_sdram() {
	uint32_t *p = (uint32_t *)psdram;
	int i;
	for (i = 0; i < (int) (SDRAM_SIZE / sizeof(p[0])); i ++) {
			p[i] = 0;
	}
	Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", SDRAM_BASE, SDRAM_BASE + SDRAM_SIZE);
	return;
}


//=========================================================================
// physical memory addr read and write
// ========================================================================
word_t paddr_read(paddr_t addr, int len) {
  // read from psram
	if (likely(in_psram(addr))) {
		word_t num = pmem_read(addr, len); 
		//printf("NPC paddr_read() :read at address = %#x, get data = %#x, len = %d, pc = %#x\n", addr,num, len, get_pc());
		if (nextpc() != addr) { // 过滤掉读指令
			log_write("		NPC: Read mem at address = %#x, data = %#x, now PC = %#x\n", addr, num, get_pc()); 
		}
		return num;
	}
	// read from sdram
  if (likely(in_psdram(addr))) {
	  if (addr >= SDRAM_BASE && addr <= SDRAM_BASE + SDRAM_SIZE) {
		  word_t num = psdram_read(addr, len); 
		  //printf("NPC sdram read, address = %#x, return num = %#x, pc = %#x\n", addr, num, get_pc());
		  return num;		
	  }
  }
	// read from flash
	if (addr >= FLASH_BASE && addr <= FLASH_BASE + FLASH_SIZE) {
		word_t num = pflash_read(addr, len); 
		//printf("NPC flash read, address = %#x, return num = %#x, pc = %#x\n", addr, num, get_pc());
		return num;		
	}
	// read for mrom
	if (addr >= 0x20000000 && addr <= 0x20000fff) {
		int idx = (addr - 0x20000000)/4;
		// 因为实际上load_img() 是将其读到 pmem 处存着的
		return *((uint32_t *)psram + idx);		
	}

	printf("read out of bound--");	
	out_of_bound(addr);
	return 0;
}

// write
void paddr_write(paddr_t addr, int len, word_t data) {
  // write to psram
  if (likely(in_psram(addr))) { 
		//printf("NPC: write at address = %#x, write data = %#x, pc = %#x\n", addr, data, get_pc());
		log_write("		NPC: Write mem at address = %#x, data = %#x, now PC = %#x\n", addr, data, get_pc()); 
		pmem_write(addr, len, data); 
		return; 
	}
  // write to sdram
  if (likely(in_psdram(addr))) { 
		//printf("NPC: write at address = %#x, write data = %#x, pc = %#x\n", addr, data, get_pc());
		log_write("		NPC: Write mem at address = %#x, data = %#x, now PC = %#x\n", addr, data, get_pc()); 
		psdram_write(addr, len, data); 
		return; 
	}
	printf("write out of bound--");	
  out_of_bound(addr);
}

//=========================================================================
// vaddr read and write 
// ========================================================================
word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}
