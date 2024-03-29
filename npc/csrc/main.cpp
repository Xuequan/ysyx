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
}

 // 仿真复位
void sim_reset(Vtop *dut, uint64_t sim_reset_time) {
	dut->rst = 0;
	while (sim_reset_time--) {
		dut->rst = 1;
		printf("%ld - sim_reset(), here\n", sim_reset_time);
		step_and_dump_wave();
	}
}

void sim_exit() {
	tfp->close();
	delete top;
	//delete tfp;
	delete contextp;
}

// 执行一个cycle or instruction
// return 1 if program ended
/*
int exec_once() {
	top->clk ^= 1;
	step_and_dump_wave();

	printf("inst = %08x\n", top->inst);

	top->check_ebreak(&a);
	if (a == 1) {
		printf("\nReach ebreak instruction, stop sim.\n\n");
		return 1;
	}

	return 0;
}
*/
/*
void execute(uint64_t n) {
	for( ; n > 0; n--) {
		if( exec_once() ) {
			printf("program ended\n");
			break;
		}
	}
}
*/

//extern uint8_t pmem;

word_t vaddr_ifetch(vaddr_t addr, int len);
// 总是读取地址为 raddr & ~0x3u 的4字节返回
// raddr 是 vaddr
extern "C" int pmem_read(int raddr) {
	printf("raddr = %#x\n", raddr);
	return vaddr_ifetch(raddr, 4); 
}
	// 总是往地址为 'waddr & ~0x3u '的4字节按写掩码 'wmask' 写入 'wdata'
	// 'wmask' 中每比特表示 'wdata' 中1个字节的掩码，
	// 如 'wmask = 0x3' 代表只写入最低2个字节，内存中的其它字节保持不变
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
	return;	
}
/*
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

void print_img(long size) {
	for(int i = 0; i < size; i++){
		if ( i != 0 && i % 4 == 0) 
			printf("\n");
		printf(" %02x", mem[i]);
	}
	printf("\n");
}

void load_img_in_main(char *image_file) {
	// 初始化 mem 
	memset(mem, 0, MEM_SIZE);
	// load program into memory 
	FILE *fp = fopen(image_file, "rb");
	assert(fp != 0);
	
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	if (size > MEM_SIZE){
		printf("load '%s' size('%ld') is too big\n", image_file, size);
		fclose(fp);
		return;  
	}
	fseek(fp, 0, SEEK_SET);
	int ret = fread(mem, size, 1, fp);
	assert(ret == 1);

	fclose(fp);
	// print image content
	print_img(size);
}
*/

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void execute(uint64_t n);

int main(int argc, char *argv[]) {

	printf("argc = %d, argv[0] = %s, argv[1] = %s\n", 
					argc, argv[0], argv[1]);
	if (argc != 2) {
		printf("Error, cannot get image file\n");
		return 0;
	}	

	/* load image into mem */
	//load_img_in_main( argv[1]);

	init_monitor(argc, argv);

	/* 初始化仿真 */
	sim_init();
	sim_reset(top, 5);
	top->rst = 0;
	top->clk ^= 1;
	
	//sdb_mainloop(); 
	/*
	for (int i = 0; i < 10; i++) {
		exec_once();
	}
	*/
	execute(10);

	sim_exit();
	return 0;
}
