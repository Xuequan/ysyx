#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
		//ev.cause = c->mcause;
    switch (c->mcause) {
			// 根据手册 p38
			case 12: ev.event = EVENT_PAGEFAULT; break;
			case 11: ev.event = EVENT_YIELD;  	 break;
			case 8 : ev.event = EVENT_SYSCALL; 	break;
			case 0x10000004: ev.event = EVENT_IRQ_TIMER; break;
			// EVENT_IRQ_IODEV	
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

/* cte_init() 做两件事：
 * 1. 设置异常入口地址
 * 2. 注册一个事件处理回调函数
 */
bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
