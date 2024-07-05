#ifndef YSYXSOC_H__
#define YSYXSOC_H__

#include <klib-macros.h>

#include "riscv/riscv.h"

// 实际上 npc_trap 还未实现，只不过现在只是遇到 ebreak 就停止仿真
#define ysyxsoc_trap(code) asm volatile("ebreak")

#define UART_BASE     0x10000000L
/*
#define DEVICE_BASE 0xa0000000

#define MMIO_BASE 0xa0000000

#define SERIAL_PORT     (DEVICE_BASE + 0x00003f8)
#define KBD_ADDR        (DEVICE_BASE + 0x0000060)
#define RTC_ADDR        (DEVICE_BASE + 0x0000048)
#define VGACTL_ADDR     (DEVICE_BASE + 0x0000100)
#define AUDIO_ADDR      (DEVICE_BASE + 0x0000200)
#define DISK_ADDR       (DEVICE_BASE + 0x0000300)
#define FB_ADDR         (MMIO_BASE   + 0x1000000)
#define AUDIO_SBUF_ADDR (MMIO_BASE   + 0x1200000)

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

#define NEMU_PADDR_SPACE \
  RANGE(&_pmem_start, PMEM_END), \
  RANGE(FB_ADDR, FB_ADDR + 0x200000), \
  RANGE(MMIO_BASE, MMIO_BASE + 0x1000) // serial, rtc, screen, keyboard 

//typedef uintptr_t PTE;

#define PGSIZE    4096

*/
#endif
