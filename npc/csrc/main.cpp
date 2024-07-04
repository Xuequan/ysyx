#include <cstdio>
#include "VysyxSoCFull.h"

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void sim_init();
void sim_exit();

int main(int argc, char *argv[]) {

	Verilated::commandArgs(argc, argv);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}

	sim_init();

	init_monitor(argc, argv);
	
	sdb_mainloop(); 

	sim_exit();
	return 0;
}
