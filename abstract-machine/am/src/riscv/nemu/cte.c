#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
<<<<<<< HEAD
  if (user_handler) {
    Event ev = {0};
		//ev.cause = c->mcause;
=======
	printf("\n__am_irq_handle()-1:  input c = %#x, c->mcause = %#x \n", c, c->mcause);
  if (user_handler) {
    Event ev = {0};
>>>>>>> tracer-ysyx
    switch (c->mcause) {
			// 根据手册 p38
			case 12: ev.event = EVENT_PAGEFAULT; break;
			case 11: ev.event = EVENT_YIELD;  	 break;
			case 8 : ev.event = EVENT_SYSCALL; 	break;
			case 0x10000004: ev.event = EVENT_IRQ_TIMER; break;
			// EVENT_IRQ_IODEV	
      default: ev.event = EVENT_ERROR; break;
    }
<<<<<<< HEAD

    c = user_handler(ev, c);
    assert(c != NULL);
  }

=======
    c = user_handler(ev, c);
    assert(c != NULL);
  }
	printf("\n__am_irq_handle()-2: return now c = %#x, c->mcause = %#x\n", c, c->mcause);
>>>>>>> tracer-ysyx
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

<<<<<<< HEAD
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
=======
/* kcontext(): 创建内核线程的上下文 */
/* input: Area kstack --> 栈的范围；
 * 				entry  --> 是内核线程的入口
 * 				arg    --> 是内核线程的参数
 * kcontext() 要求内核线程不能从 entry 返回；
 * todo: 需要在 kstack 的底部创建一个以 entry 为入口的上下文
 * 			 结构，然后返回这一结构的指针。
 */
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
	Context** cp = (Context** ) kstack.start;
	Context* ctx = (Context* )((uint8_t*)kstack.end - sizeof(Context) );
	*cp = ctx;
	ctx->mcause = (uintptr_t)0xb;
	ctx->mepc = (uintptr_t)entry;
	ctx->gpr[10] = (uintptr_t)arg;  // a0

	printf("\nkcontext(), Area(%#x -- %#x), c = %#x\n", kstack.start, kstack.end, ctx);

	return ctx;
>>>>>>> tracer-ysyx
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
