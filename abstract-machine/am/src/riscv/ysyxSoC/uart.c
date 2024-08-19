#include <am.h>
#include <ysyxsoc.h>
#include <klib.h>

void __am_uart_rx(AM_UART_RX_T *rec) {
  uint8_t t = inb((uintptr_t)(UART_BASE + UART_RX)); 
  if ( t == 0)
    rec->data = 0xff;
  else
    rec->data = (char)t;

  //outb((uintptr_t)(UART_BASE + UART_TX), rec->data);
}

void __am_uart_tx(AM_UART_TX_T *rec) {
  outb((uintptr_t)(UART_BASE + UART_TX), rec->data);
}

