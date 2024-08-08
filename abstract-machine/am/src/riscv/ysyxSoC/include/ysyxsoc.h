#ifndef YSYXSOC_H__
#define YSYXSOC_H__

#include <klib-macros.h>

#include "riscv/riscv.h"

#define ysyxsoc_trap(code) asm volatile("ebreak")

// SRAM 只有 8kb 
#define PMEM_SIZE (8 * 1024 * 8)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

// -----------------------------------------------
// some peril address space 
// ------------------------------------------------
// timer 映射到 clint
#define RTC_ADDR 0x02000000L

// ps2-keyboard
#define PS2_KEYBOARD 0x10011000L

// uart 
#define UART_RX   0
#define UART_BASE 0x10000000L
#define UART_TX 0    
#define UART_LC 3   // line control register
#define UART_DL1 0  // divisor latch low byte
#define UART_DL2 1  // divisor latch high byte
#define UART_LS 5   // line status register 

// -----------------------------------------------
// extern val from linker scripts
// ------------------------------------------------
extern char _pmem_start;

extern char _heap_start;
extern char _heap_end;

extern char _sbss[];
extern char _ebss[];
extern char _data_start[];
extern char _data_end[];
extern char _data_load_addr[];

extern char _text_load_addr[];
extern char _text_start[];
extern char _text_end[];

extern char _rodata_load_addr[];
extern char _rodata_start[];
extern char _rodata_end[];

extern char _ssbl_load_addr[];
extern char _sssbl[];
extern char _essbl[];

#ifdef RUN_RTT
// from rt-thred-am/bsp/abstract-machine/extra.ld
extern char __fsymtab_start[];
extern char __am_apps_data_end[];
extern char __data_extra_load_addr[];
extern char __am_apps_bss_start[];
extern char __am_apps_bss_end[];
#endif


#endif
