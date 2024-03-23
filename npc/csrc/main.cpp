#include "Vtop.h"
#include <stdlib.h>
//#include "nvboard.h"
#include "verilated_vcd_c.h"
#include "verilated.h"

#include <stdio.h>

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <fstream>
using namespace std;

#include "Vtop__Dpi.h"
#include "svdpi.h"

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

static Vtop* top;

char *file = "/home/chuan/ysyx-workbench/npc/csrc/ram.txt";
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
/*
void nvboard_bind_all_pins(Vtop *top) {
	nvboard_bind_pin(&top->a, false, true, 1, LD0);
	nvboard_bind_pin(&top->b, false, true, 1, LD1);
	nvboard_bind_pin(&top->f, false, true, 1, LD2);
}
*/

// 读入指令
map<string, string> instructions;
//map<string, unsigned int> instructions;
void ram_init(void) {
	ifstream infile;
	infile.open(file);
	if (! infile) {
		printf("Open ram.txt wrong!\n");
		return;
	}
	string __addr, addr;
	string inst;
	string line;
	while (getline(infile, line) ){
		istringstream stream(line);
		stream >> __addr;
		addr = __addr.substr(0, 8);
		addr = "0x" + addr;
		stream >> inst;
		instructions[addr] = inst;
	}
	infile.close();
}

void print_instructions() {
	printf("==============================\n");
	map<string, string>::iterator it = instructions.begin();
	for ( ; it != instructions.end(); it++)
		cout << it->first << ": " << it->second << endl;
		//printf("%#x, %#x\n", it->first, it->second);
	printf("==============================\n");
}

extern "C" int pmem_read(int raddr) {
	/* todo */
	// 总是读取地址为 raddr & ~0x3u 的4字节返回

}
extern "c" void pmem_write(int waddr, int wdata, char wmask) {
	// 总是往地址为 'waddr & ~0x3u '的4字节按写掩码 'wmask' 写入 'wdata'
	// 'wmask' 中每比特表示 'wdata' 中1个字节的掩码，
	// 如 'wmask = 0x3' 代表只写入最低2个字节，内存中的其它字节保持不变
}

unsigned int pmem_read(unsigned int addr, bool *success) {
	printf("pmem_read(): input addr = %#x\n", addr);
	map<string, string>::iterator it;
	it = instructions.begin();
	char buf[12];
	for ( ; it != instructions.end(); ++it) {
		string tmp = it->first;
		//printf("pmem_read() :\n");
		unsigned int tmp2 = (unsigned int)strtol(tmp.c_str(), NULL, 16);
		/*
		printf("------%#x\n", tmp2);
		int i = 0;
		for (auto c : it->first)
		{
			buf[i] = c;
			i++;
		}
		buf[i] = '\0';
		printf("== %#x\n", *(unsigned int *)buf);
		if (*(unsigned int *)buf == addr)
			break; 
		*/
		if (tmp2 == addr)
			break;
	}

	if (it == instructions.end() ){
		printf("Cannot find a instruction at address %#x\n", addr);
		
		printf("\n");
		*success = 0;
		return 0;
	}
	string inst = it->second;
	unsigned int ret;
	sscanf(inst.c_str(), "%x", &ret);
	*success = 1;
	return ret;
}


int main(int argc, char *argv[]) {
	sim_init();
	ram_init();	
	print_instructions();
	
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
		
		bool success = 0;

		if (top->clk) { 
			top->inst = pmem_read((unsigned int)top->pc, &success);
			if (!success)	{
				printf("Failed to get pc at %#x\n", top->pc);
				break;
			}

			top->check_ebreak(&a);
			if (a == 1) {
				printf("Reach ebreak instruction, stop sim.\n");
				step_and_dump_wave();
				break;
			}

			printf("%d: top->clk = %d, top->pc = %#x, top->inst= %#x \n", i, top->clk, top->pc, top->inst);
		}

		step_and_dump_wave();
	} // end while(1)
	sim_exit();
	return 0;
}
