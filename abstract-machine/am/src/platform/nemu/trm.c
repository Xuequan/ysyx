#include <am.h>
#include <nemu.h>   //am/src/platform/nemu/inc/

extern char _heap_start;
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END);

#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
  outb(SERIAL_PORT, ch);
}

void halt(int code) {
  nemu_trap(code);

  // should not reach here
  while (1);
}

void _trm_init() {
	/* 不确定是不是加在这里，暂且试一试吧 */
	/*
	if (ioe_init() == false) {
		panic("ioe_init() error\n");  // from klib-macro.h
	}
	*/

  int ret = main(mainargs);
  halt(ret);
}
