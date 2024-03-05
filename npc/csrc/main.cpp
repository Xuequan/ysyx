#include "Vtop.h"
#include <stdlib.h>
//#include "nvboard.h"
#include "verilated_vcd_c.h"
#include "verilated.h"

#include <stdio.h>
//#include "/home/chuan/ysyx-workbench/nemu/include/memory"

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <fstream>
using namespace std;

// static TOP_NAME dut;

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
/*
void nvboard_bind_all_pins(Vtop *top) {
	nvboard_bind_pin(&top->a, false, true, 1, LD0);
	nvboard_bind_pin(&top->b, false, true, 1, LD1);
	nvboard_bind_pin(&top->f, false, true, 1, LD2);
}
*/

// 读入指令
map<unsigned int, string> instructions;
//map<unsigned int, unsigned int> instructions;
void ram_init(void) {
	ifstream infile;
	infile.open("/home/chuan/ysyx-workbench/npc/csrc/ram.txt");
	if (! infile) {
		printf("Open ram.txt wrong!\n");
		return;
	}
	string __addr, _addr;
	unsigned int addr;

	string inst;
	string line;
	while (getline(infile, line) ){
		istringstream stream(line);
		stream >> __addr;

		_addr = __addr.substr(0, 8);
		istringstream st(_addr);
		st >> addr;		

		stream >> inst;
		instructions[addr] = inst;
	}
	infile.close();
}
void print_instructions() {
	map<unsigned int, string>::iterator it = instructions.begin();
	for ( ; it != instructions.end(); it++)
		cout << it->first << ": " << it->second << endl;
}
unsigned int pmem_read(unsigned int addr) {
	map<unsigned int, string>::iterator it;
	it = instructions.find(addr);
	if (it == instructions.end()) {
		cout << "Cannot find a instruction at address " << addr << "\n";
		return 0;
	}
	string inst = instructions[addr];
	cout << "==== inst = " << inst <<endl;
	unsigned int ret;
	sscanf(inst.c_str(), "%x", &ret);
	printf("inst = %#x\n", ret);

	unsigned int test;
	istringstream ss1(inst);
	ss1 >> test;
	cout << "test1 :" << test << endl;	
	return ret;
}


int main() {
	sim_init();

	/*
	nvboard_bind_all_pins(top);
	nvboard_init();
	*/
	ram_init();	
	print_instructions();
	for (int i = 0; i < 10; i++)
		top->rst = 1;	
	top->rst = 0;
	step_and_dump_wave();
	top->clk = 1;
/*
	top->pc = (unsigned int)80000000;
	top->inst = pmem_read(top->pc);
	cout << "get inst: " << top->inst << endl;
	step_and_dump_wave();
*/

	/*
	while (1) {
		if (top->pc == 0x80000004)
			break;
		top->clk = ~top->clk;	
		top->inst = pmem_read(top->pc);
		step_and_dump_wave();
		//nvboard_update();
	}
	*/
	//nvboard_quit();
	
	sim_exit();
	return 0;
}
