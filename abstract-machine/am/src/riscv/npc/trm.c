#include <am.h>
#include <klib-macros.h>
#include <klib.h>

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
	printf("here in npc/trm.c \n");
}

// 实际上的 npc_trap() 还未实现，现在只不过是遇到 ebreak 就停止了罢了
#define npc_trap(code) asm volatile("ebreak")
void halt(int code) {
	npc_trap(code);

  while (1) {
	}
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
