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
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cinttypes>
#include "common2.h"

uint8_t* guest_to_host(paddr_t paddr); 

paddr_t host_to_guest(uint8_t *haddr);

static inline word_t host_read(void *addr, int len);

static inline void host_write(void *addr, int len, word_t data);

static word_t pmem_read(paddr_t addr, int len); 

static void pmem_write(paddr_t addr, int len, word_t data);

static void out_of_bound(paddr_t addr);

void init_mem();

word_t paddr_read(paddr_t addr, int len);

<<<<<<< HEAD
word_t vaddr_ifetch(vaddr_t addr, int len);

=======
>>>>>>> tracer-ysyx
word_t vaddr_read(vaddr_t addr, int len);

void paddr_write(paddr_t addr, int len, word_t data);

void vaddr_write(vaddr_t addr, int len, word_t data);

#endif
