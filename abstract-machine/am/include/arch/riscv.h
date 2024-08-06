#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

<<<<<<< HEAD
struct Context {
  // TODO: fix the order of these members to match trap.S
  //uintptr_t mepc, mcause, gpr[NR_REGS], mstatus;
  /*
  uint32_t gpr[1], gpr[3], gpr[4], gpr[5], gpr[6], gpr[7];
	uint32_t gpr[8], gpr[9], gpr[10], gpr[11], gpr[12];
	uint32_t gpr[13], gpr[14], gpr[15];
	uint32_t mcause, mstatus, mepc;
	*/
=======
#define NR_PARAMS 3
struct Context {
  // TODO: fix the order of these members to match trap.S
  //uintptr_t mepc, mcause, gpr[NR_REGS], mstatus;
>>>>>>> tracer-ysyx
	uintptr_t gpr[NR_REGS], mcause, mstatus, mepc;
  void *pdir;
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif
