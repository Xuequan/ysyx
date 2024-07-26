#include <cstdio>
#include "VysyxSoCFull.h"

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void sim_init();
void sim_exit();

void print_flash();

int main(int argc, char *argv[]) {

	Verilated::commandArgs(argc, argv);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}

	sim_init();

	init_monitor(argc, argv);
	
	/* for test only */
	print_flash();

	sdb_mainloop(); 

	sim_exit();
	return 0;
}
