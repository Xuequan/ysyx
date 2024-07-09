#include <am.h>
#include <ysyxsoc.h>

extern char _heap_start;
extern char _heap_end;
int main(const char *args);

Area heap = RANGE(&_heap_start, &_heap_end);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
	outb(UART_BASE, '$');
	outb(UART_BASE, ch);
}

void halt(int code) {
	ysyxsoc_trap(code);

  while (1) {
	}
}

void _trm_init() {
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
