#include <am.h>
#include <ysyxsoc.h>
#include <klib.h>

void __am_uart_receive(AM_UART_RX_T *rec) {
  // 只有一个成员 char rec->data;
  uint8_t t = inb((uintptr_t)(UART_BASE + UART_RX)); 
  if ( t == 0)
    rec->data = 0xff;
  else
    rec->data = (char)t;
}
