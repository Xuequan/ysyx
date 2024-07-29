// 本文件对应于 soc_linkerv5.ld 
// 实现二级 bootloader
#include <am.h>
#include <ysyxsoc.h>

#define UART_BASE 0x10000000L
#define UART_TX 0    
#define UART_LC 3   // line control register
#define UART_DL1 0  // divisor latch low byte
#define UART_DL2 1  // divisor latch high byte
#define UART_LS 5   // line status register 

extern char _heap_start;
extern char _heap_end;
int main(const char *args);

Area heap = RANGE(&_heap_start, &_heap_end);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

/* initial uart  */
void init_uart() {
	// set lcr[7] 1
	*(volatile uint8_t *)(UART_BASE + UART_LC) = *(volatile uint8_t *)(UART_BASE + UART_LC) | 0x80; 
	// set divisor latch register 
	*(volatile uint8_t *)(UART_BASE + UART_DL1) = (uint8_t)0x60;
	*(volatile uint8_t *)(UART_BASE + UART_DL2) = (uint8_t)0x00;
	// set lcr[7] 0
	*(volatile uint8_t *)(UART_BASE + UART_LC) = *(volatile uint8_t *)(UART_BASE + UART_LC) & 0x7f; 
}

void putch(char ch) {
	// get lsr[6] 
	uint8_t lsr6 = *(volatile uint8_t *)(UART_BASE + UART_LS) & 0b01000000;  
	int i = 0;
	while (lsr6 == 0) {
	  // wait
		if ( i == 100) i = 0;
		else 					 i++;
		lsr6 = *(volatile uint8_t *)(UART_BASE + UART_LS) & 0b01000000;  
	}
	*(volatile char *)(UART_BASE + UART_TX) = ch;
}

void halt(int code) {
	ysyxsoc_trap(code);
  while (1) {
	}
}

void _trm_init() {
	init_uart();
  int ret = main(mainargs);
  halt(ret);
}

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

// from rt-thred-am/bsp/abstract-machine/extra.ld
extern char __fsymtab_start[];
extern char __am_apps_data_end[];
extern char __data_extra_load_addr[];
extern char __am_apps_bss_start[];
extern char __am_apps_bss_end[];
void __attribute__  ((section (".ssbl"))) _ss_bootloader() {
	char *dst;
	char *src; 
  // zero .bss
	for (dst = _sbss; dst < _ebss; dst++)
		*dst = 0;
  // zero .bss.extra
  for (dst = __am_apps_bss_start; dst < __am_apps_data_end; dst++)
    *dst = 0;

	// copy '.data' section to psram
	src = _data_load_addr;
	dst = _data_start;
	while (dst < _data_end)
		*dst++ = *src++;
	// copy '.text' section to sram
	src = _text_load_addr;
	dst = _text_start;
	while (dst < _text_end)
		*dst++ = *src++;
	// copy '.rodata' section to sram
	src = _rodata_load_addr;
	dst = _rodata_start;
	while (dst < _rodata_end)
		*dst++ = *src++;

	src = __data_extra_load_addr;
	dst = __fsymtab_start;
	while (dst < __am_apps_data_end)
		*dst++ = *src++;

  _trm_init();
}

// fsbl loads ssbl code from flash to sram
extern char _ssbl_load_addr[];
extern char _sssbl[];
extern char _essbl[];

void __attribute__  ((section (".fsbl"))) _fs_bootloader() {
	char *dst;
	char *src; 
	src = _ssbl_load_addr;
	dst = _sssbl;
	while (dst < _essbl)
		*dst++ = *src++;

  _ss_bootloader();
}
