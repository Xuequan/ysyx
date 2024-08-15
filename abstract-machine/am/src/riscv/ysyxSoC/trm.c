#include <am.h>
#include <ysyxsoc.h>

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

void __attribute__  ((section (".ssbl"))) _ss_bootloader() {
	char *dst;
	char *src; 
  // zero .bss
	for (dst = _sbss; dst < _ebss; dst++)
		*dst = 0;

#ifdef RUN_RTT
  // zero .bss.extra
  for (dst = __am_apps_bss_start; dst < __am_apps_data_end; dst++)
    *dst = 0;
#endif

	// copy '.data'
	src = _data_load_addr;
	dst = _data_start;
	while (dst < _data_end)
		*dst++ = *src++;
	// copy '.text' 
	src = _text_load_addr;
	dst = _text_start;
	while (dst < _text_end)
		*dst++ = *src++;
	// copy '.rodata'
	src = _rodata_load_addr;
	dst = _rodata_start;
	while (dst < _rodata_end)
		*dst++ = *src++;

#ifdef RUN_RTT
	src = __data_extra_load_addr;
	dst = __fsymtab_start;
	while (dst < __am_apps_data_end)
		*dst++ = *src++;
#endif

  _trm_init();
}

// fsbl loads ssbl code from flash to sram
void __attribute__  ((section (".fsbl"))) _fs_bootloader() {
	char *dst;
	char *src; 
	src = _ssbl_load_addr;
	dst = _sssbl;
	while (dst < _essbl)
		*dst++ = *src++;

  _ss_bootloader();
}
