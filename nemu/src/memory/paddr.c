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
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

extern word_t pflash_read(paddr_t addr, int len);
extern word_t pmrom_read(paddr_t addr, int len);
extern word_t psram_read(paddr_t addr, int len);
extern void psram_write(paddr_t addr, int len, word_t data);

 // 在 nemu 中运行的程序称为 "guest 程序”
 // guest_to_host() 将 paddr 转化为 nemu 模拟的计算机地址在
 // 本电脑上的地址
uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
	// host_to_guest() 则相反
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
	#ifdef CONFIG_DIFFTEST
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
	#endif
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
#ifdef CONFIG_MEM_RANDOM
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
  	p[i] = rand();
  }
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

word_t paddr_read(paddr_t addr, int len) {
	// mrom
  if (likely(in_mrom(addr))) { 
		word_t num = pmrom_read(addr, len); 
#ifdef CONFIG_MTRACE
		if (cpu.pc != addr)  // fliter instruction fetch
			log_write("Read from mrom: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, num, cpu.pc); 
#endif
		return num; 
	}
	// flash
  if (likely(in_flash(addr))) { 
		word_t num = pflash_read(addr, len); 
#ifdef CONFIG_MTRACE
		if (cpu.pc != addr)  // fliter instruction fetch
			log_write("Read from flash: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, num, cpu.pc); 
#endif
		return num; 
	}
	// mem
  if (likely(in_pmem(addr))) {
		word_t num = pmem_read(addr, len); 
		printf("NEMU: read address = %#x, get data = %#x, pc = %#x\n\n", addr, num, cpu.pc); 
		log_write("Read from mem: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, num, cpu.pc); 
#ifdef CONFIG_MTRACE
		if (cpu.pc != addr)  // fliter instruction fetch
			log_write("Read from mem: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, num, cpu.pc); 
#endif
		return num;
	}
	// sram
  if (likely(in_sram(addr))) { 
		word_t num = psram_read(addr, len); 
#ifdef CONFIG_MTRACE
		if (cpu.pc != addr)  // fliter instruction fetch
			log_write("Read from sram: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, num, cpu.pc); 
#endif
		return num; 
	}

  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
	// 若是NEMU作为NPC的 ref，那么 CONFIG_DEVICE 也没有，那么对于I/O 就会执行到这里
	// 进而在NPC显示里报错
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { 
		printf("NEMU: write address = %#x, write data = %#x, pc = %#x\n\n", addr, data, cpu.pc); 
#ifdef CONFIG_MTRACE
		log_write("Write to mem: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, data, cpu.pc); 
#endif
		pmem_write(addr, len, data); 
		return; 
	}
	// mrom 
  if (likely(in_mrom(addr))) { 
#ifdef CONFIG_MTRACE
		log_write("Write to mrom: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, data, cpu.pc); 
#endif
		printf("NEMU: please check, mrom cannot write after init\n");
		return; 
	}
	
	// sram
  if (likely(in_sram(addr))) { 
#ifdef CONFIG_MTRACE
		log_write("Write to sram: address = %#x, length = %d, data = %#x, pc = %#x\n", addr, len, data, cpu.pc); 
#endif
		psram_write(addr, len, data); 
		return; 
	}

  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
