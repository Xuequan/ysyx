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

void *memcpy(void*, const void *, size_t);

extern char _rodata_start[];
extern char _rodata_size[];
extern char _rodata_load_start[];
extern char _data_start[];
extern char _data_size[];
extern char _data_load_start[];
extern char _bss_start[];
extern char _bss_size[];
extern char _bss_load_start[];

void copy_data(void *dst, void *src, size_t n) {
	if ((char *)dst != (char *)src) 
		memcpy(dst, src, n); 
}

void _trm_init() {
	//copy_data(_rodata_start, _rodata_load_start, (size_t)_rodata_size);		
	//copy_data(_data_start, _data_load_start, (size_t)_data_size);		
	copy_data(_bss_start, _bss_load_start, (size_t)_bss_size);		

  int ret = main(mainargs);
  halt(ret);
}
