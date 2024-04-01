#include <cstdio>
#include "common2.h"

/*
word_t vaddr_ifetch(vaddr_t addr, int len);
// 总是读取地址为 raddr & ~0x3u 的4字节返回
// raddr 是 vaddr
extern "C" int pmem_read(int raddr) {
  return vaddr_ifetch(raddr, 4); 
}
  // 总是往地址为 'waddr & ~0x3u '的4字节按写掩码 'wmask' 写入 'wdata'
  // 'wmask' 中每比特表示 'wdata' 中1个字节的掩码，
  // 如 'wmask = 0x3' 代表只写入最低2个字节，内存中的其它字节保持不>变
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  return; 
}   
*/

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void sim_init();
void sim_exit();

int main(int argc, char *argv[]) {

	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}

	init_monitor(argc, argv);

	sim_init();
	
	sdb_mainloop(); 

	sim_exit();
	return 0;
}
