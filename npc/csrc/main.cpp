#include <cstdio>
<<<<<<< HEAD
=======
#include "VysyxSoCFull.h"
#include "ctrl.h"
>>>>>>> tracer-ysyx

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void sim_init();
void sim_exit();
<<<<<<< HEAD

int main(int argc, char *argv[]) {

=======
void destory_mem();


int main(int argc, char *argv[]) {

	Verilated::commandArgs(argc, argv);
>>>>>>> tracer-ysyx
	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}

	sim_init();

	init_monitor(argc, argv);
	
<<<<<<< HEAD
	sdb_mainloop(); 

	sim_exit();
=======
#ifdef PRINT_FLASH_MEM 
	/* for test only */
  void print_flash();
  void print_mem();
	print_flash();
	print_mem();
#endif

	sdb_mainloop(); 

	sim_exit();
	
	destory_mem();
>>>>>>> tracer-ysyx
	return 0;
}
