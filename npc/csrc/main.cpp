#include "Vtop.h"
#include <cstdlib>
#include "verilated_vcd_c.h"
#include "verilated.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdint>

#include "Vtop__Dpi.h"
#include "svdpi.h"

#include "common.h"

Vtop* top;
VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;
svBit a; 

void step_and_dump_wave() {
	top->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}
void sim_init() {
	contextp = new VerilatedContext;
	tfp = new VerilatedVcdC;
	top = new Vtop;
	contextp->traceEverOn(true);

	top->trace(tfp, 0);
	tfp->open("dump.vcd");

	/* DPI-C 接口 */
	const svScope scope = svGetScopeFromName("TOP.top");
	assert(scope);
	svSetScope(scope);

	int i = -1;
	while ( i < 5) {
		i++;
		top->clk ^= 1;
		top->rst = 1;
		step_and_dump_wave();
	}
	top->rst = 0;
}

void sim_exit() {
	int i = 2;
	while (i--) {	
		printf("top->clk = %d\n", top->clk);
		top->rst = 1;
	}
	tfp->close();
	delete top;
	delete contextp;
}

word_t vaddr_ifetch(vaddr_t addr, int len);
// 总是读取地址为 raddr & ~0x3u 的4字节返回
// raddr 是 vaddr
extern "C" int pmem_read(int raddr) {
	return vaddr_ifetch(raddr, 4); 
}
	// 总是往地址为 'waddr & ~0x3u '的4字节按写掩码 'wmask' 写入 'wdata'
	// 'wmask' 中每比特表示 'wdata' 中1个字节的掩码，
	// 如 'wmask = 0x3' 代表只写入最低2个字节，内存中的其它字节保持不变
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
	return;	
}

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void execute(uint64_t n);

int main(int argc, char *argv[]) {

	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}

	init_monitor(argc, argv);

	sim_init();
	top->clk ^= 1;
	
	sdb_mainloop(); 
	//execute(30);

	sim_exit();
	return 0;
}
