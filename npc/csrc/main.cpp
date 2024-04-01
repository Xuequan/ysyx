#include <cstdio>

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
