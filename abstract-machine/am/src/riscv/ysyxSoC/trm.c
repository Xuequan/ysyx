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


extern char _data_start[];
extern char _data_end[];
extern char _data_load_addr[];

extern char _text_load_addr[];
extern char _text_start[];
extern char _text_end[];

extern char _rodata_load_addr[];
extern char _rodata_start[];
extern char _rodata_end[];

void __attribute__  ((section (".copy_to_psram"))) _data_init() {
	char *dst;
	char *src; 
	// copy '.data' section to psram
	src = _data_load_addr;
	dst = _data_start;
	while (dst < _data_end)
		*dst++ = *src++;
    /*
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
  */
}

/* zero bss  */
extern char _sbss[];
extern char _ebss[];
void __attribute__ ((section(".zero_bss"))) _zero_bss() {
	char *dst;
	for (dst = _sbss; dst < _ebss; dst++)
		*dst = 0;
}

void _trm_init() {

	_zero_bss();
	_data_init();

	init_uart();

  int ret = main(mainargs);
  halt(ret);
}

