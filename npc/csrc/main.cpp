#include <Vtop.h>
#include <stdlib.h>
#include "nvboard.h"

static TOP_NAME dut;

void nvboard_bind_all_pins(Vtop *top) {
	nvboard_bind_pin(&top->a, false, true, 1, LD0);
	nvboard_bind_pin(&top->b, false, true, 1, LD0);
	nvboard_bind_pin(&top->f, false, false, 1, LD0);
}

int main() {
	nvboard_bind_all_pins(&dut);
	nvboard_init();
	while (1) {
		dut.a = rand() & 1;
		dut.b = rand() & 1;
		dut.eval();
		nvboard_update();
	}
	nvboard_quit();
	return 0;
}
