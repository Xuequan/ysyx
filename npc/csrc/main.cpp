#include "Vtop.h"
#include <stdlib.h>
//#include "nvboard.h"
#include "verilated_vcd_c.h"
#include "verilated.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>


#include "Vtop__Dpi.h"
#include "svdpi.h"

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

static Vtop* top;

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
}
void sim_exit() {
	tfp->close();
}

#define MEM_BASE 0x80000000
#define MEM_SIZE 1024
static uint8_t mem[MEM_SIZE];  // memory
// 总是读取地址为 raddr & ~0x3u 的4字节返回
// raddr 是 vaddr
extern "C" int pmem_read(int raddr) {
	int ret = 0;
	memcpy(&ret, (mem + (uint8_t)(raddr - MEM_BASE) ), sizeof(int));
	return ret; 
}
	// 总是往地址为 'waddr & ~0x3u '的4字节按写掩码 'wmask' 写入 'wdata'
	// 'wmask' 中每比特表示 'wdata' 中1个字节的掩码，
	// 如 'wmask = 0x3' 代表只写入最低2个字节，内存中的其它字节保持不变
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
	return;	
}

int main(int argc, char *argv[]) {
	printf("argc = %d, argv[0] = %s, argv[1] = %s\n", argc, argv[0], argv[1]);
	if (argc != 2) {
		printf("Error, cannot get image file\n");
		return 0;
	}	
	/* 初始化 mem */
	memset(mem, 0, MEM_SIZE);
	/* load program into memory */	
	//char *image = argv[1];
	FILE *fp = fopen(argv[1], "rb");
	assert(fp != 0);
	
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0, SEEK_SET);
	int ret = fread(mem, size, 1, fp);
	assert(ret == 1);

	fclose(fp);
	/* load mem end */		

	sim_init();
	
	const svScope scope = svGetScopeFromName("TOP.top");
	assert(scope);
	svSetScope(scope);
	svBit a; 

	int i = -1;
	while (1) {
		i++;
		top->clk = i % 2;
		if (i < 5) { 
			top->rst = 1;	
			step_and_dump_wave();
			continue;
		} else { 
			top->rst = 0;	
		}
		
		if (top->clk) { 
			top->check_ebreak(&a);
			if (a == 1) {
				printf("Reach ebreak instruction, stop sim.\n");
				step_and_dump_wave();
				break;
			}
		}

		step_and_dump_wave();
	} // end while(1)
	sim_exit();
	return 0;
}
