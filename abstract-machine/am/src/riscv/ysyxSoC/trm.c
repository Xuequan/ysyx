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

void init_uart() {
	uint8_t lc = *(volatile uint8_t *)(UART_BASE + UART_LC); 
	lc &= 0b11111111; 
	// set lcr[7] 1
	*(volatile uint8_t *)(UART_BASE + UART_LC) = lc; 
	*(volatile uint8_t *)(UART_BASE + UART_DL1) = (uint8_t)0x0c;
	*(volatile uint8_t *)(UART_BASE + UART_DL2) = (uint8_t)0x00;

	// set lcr[7] 0
	lc = *(volatile uint8_t *)(UART_BASE + UART_LC); 
	*(volatile uint8_t *)(UART_BASE + UART_LC) = lc & 0b01111111;
}

void putch(char ch) {
	int i = 0;
	uint8_t lsr = *(volatile uint8_t *)(UART_BASE + UART_LS);
	uint8_t lsr6 = lsr & 0b01000000;  
	while (lsr6 == 0) {

		if ( i == 100) 
			i = 0;
		else
			i++;

		lsr = *(volatile uint8_t *)(UART_BASE + UART_LS);
  	lsr6 = lsr & 0b01000000;
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

extern char _data_start[];
extern char _data_Load_addr[];
extern char _data_end[];
extern char _bss_start[];
extern char _bss_end[];

void __attribute__  ((section (".first_init"))) _data_init() {
	char *dst;
	/* Zero bss */
	for (dst = _bss_start; dst < _bss_end; dst++)
		*dst = 0;

	char *src = _data_Load_addr;
	dst = _data_start;
	/* copy '.data' srction to sram  */
	while (dst < _data_end)
		*dst++ = *src++;

	_trm_init();
}
