#define UART_BASE 0x10000000L
#define UART_TX 0
__attribute__((__aligned__(4))) void char_test() {
	*(volatile char *)(UART_BASE + UART_TX) = 'A';
	*(volatile char *)(UART_BASE + UART_TX) = '\n';
	//asm volatile("ebreak");
	//while(1);
}
