/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

/* handle instruction LH, need sign-extends to 32-bits before storing in rd */
#define HANDLE_LH(src1, imm, rd) { \
	handle_lh(src1, imm, rd); \
}
static void handle_lh(word_t src1, word_t imm, int rd) {
	word_t ret = Mr(src1 + imm, 2);
	int sext_ret = ((int)ret << 16 ) >> 16;
	R(rd) = (word_t)sext_ret;
}

/* handle instruction MULH, before multiplication, src1 & src2 should be type casted to sword_t */
#define HANDLE_MULH(src1, src2, rd) { \
	handle_mulh(src1, src2, rd);\
}
static void handle_mulh(word_t src1, word_t src2, int rd) {
	long long result = (long long) ((sword_t)src1) * (long long) ((sword_t)src2);
	R(rd) = (word_t) (result >> 32);
}

#define HANDLE_MULHU(src1, src2, rd) { \
	handle_mulhu(src1, src2, rd);\
}
static void handle_mulhu(word_t src1, word_t src2, int rd) {
	unsigned long long result = (unsigned long long)src1 * (unsigned long long)src2;
	R(rd) = (word_t)(result >> 32);
}
#define HANDLE_MULHSU(src1, src2, rd) { \
	handle_mulhsu(src1, src2, rd);\
}
static void handle_mulhsu(word_t src1, word_t src2, int rd) {
	long long result = (long long)src1 * (unsigned long long)src2;
	R(rd) = (sword_t)(result >> 32);
}

#define HANDLE_ECALL(pc) { \
	handle_ecall(pc); \
}

static void handle_ecall(vaddr_t pc) {
	isa_raise_intr(0, pc);
}
#define HANDLE_CSRRW(src1, rd, csr) { \
	handle_csrrw(src1, rd, csr); \
}
static void handle_csrrw(word_t src1, int rd, word_t csr){
	if (csr == 0x341) {// mepc	
		printf("its mepc\n");
		R(rd) = cpu.mepc;
		cpu.mepc = src1;
	} else if (csr == 0x342) { // mcause
		printf("its mcause\n");
		R(rd) = cpu.mcause;
		cpu.mcause = src1;
	} else if (csr == 0x305) { // mtvec
		printf("its mtvec\n");
		R(rd) = cpu.mtvec;
		cpu.mtvec = src1;;
	} else {
		printf("Have not implementd '%#x' CSR\n", csr);
		return;
	}
}

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_I_JALR, TYPE_R, TYPE_B,
	TYPE_I_CSRRW,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)

#define immJ() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 20 ) | \
														BITS(i, 30, 21) << 1 | (BITS(i, 20, 20) << 11) | \
													 (BITS(i, 19, 12) << 12); } while(0)

#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12 ) | \
														BITS(i, 30, 25) << 5 | (BITS(i, 11, 8) << 1) | \
													 (BITS(i, 7, 7) << 11); } while(0)
/* update s->dnpc */
#define updateDnpc() do { s->dnpc = s->pc + *imm; } while(0)
#define updateDnpc2() do { s->dnpc = *src1 + *imm; } while(0)

/* set address least-significant bit 0 */
#define setLSBZero() do { s->dnpc &= 0xfffffffe;} while(0)  


static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int *shamt, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
	*shamt  = BITS(i, 24, 20);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
		case TYPE_J: immJ(); 		 updateDnpc(); break; 
    case TYPE_I_JALR: src1R();  immI(); updateDnpc2(); setLSBZero();   break;
    case TYPE_R: src1R(); src2R(); 			   break;
    case TYPE_B: src1R(); src2R(); immB(); break;
		// CSRRW, imm is csr
    case TYPE_I_CSRRW: src1R();    *imm = BITS(i, 31, 20); break;
  }
}


static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  int shamt = 0;
  s->dnpc = s->snpc;   

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, &shamt, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , N, HANDLE_ECALL(s->pc) ); 
	// start add instructions
	// addi 
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi	 , I, R(rd) = src1 + imm);
	// jal
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal		 , J, R(rd) = s->pc + 4); 
	// sw 
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw		 , S, Mw(src1 + imm, 4, src2)); 
	// sh
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh		 , S, Mw(src1 + imm, 2, src2)); 
	// lw 
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw		 , I, R(rd) = Mr(src1 + imm, 4)); 
	// lh 
  //INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh		 , I, R(rd) = Mr(src1 + imm, 2)); 
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh		 , I,HANDLE_LH(src1, imm, rd)); 
	// lhu
  //INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu		 , I, HANDLE_LHU(src1, imm, rd) ); 
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu		 , I, R(rd) = Mr(src1 + imm, 2)); 
	// lb
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb   	 , I, R(rd) = Mr(src1 + imm, 1));
	// jalr
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr	 , I_JALR, R(rd) = s->pc + 4); 
	// add
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2); 
	// sub
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2); 
	// slt
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = ((sword_t)src1 < (sword_t)src2) ? 1 : 0); 
	// sltu
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = (src1 < src2) ? 1 : 0); 
	// sll
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1 << BITS(src2, 4, 0)); 
	// srl
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = src1 >> BITS(src2, 4, 0)); 
	// sra
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = (sword_t)src1 >> BITS(src2, 4, 0)); 
	// and
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2); 
	// or
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2); 
	// xor
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2); 
	// slti
	INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti	 , I, R(rd) = (sword_t)src1 < (sword_t)imm ? 1 : 0);
	// sltiu
	INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu	 , I, R(rd) = src1 < imm ? 1 : 0);
	// ori
	INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori	 	 , I, R(rd) = src1 | imm);
	// xori
	INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori	 , I, R(rd) = src1 ^ imm);
	// andi
	INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi	 , I, R(rd) = src1 & imm);
	// beq
	INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq	   , B, s->dnpc = (src1 == src2) ? s->pc + imm : s->dnpc);
	// bne
	INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne	   , B, s->dnpc = ((sword_t)src1 != (sword_t)src2) ? s->pc + imm : s->dnpc);
	// blt
	INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt	   , B, s->dnpc = ((sword_t)src1 < (sword_t)src2) ? s->pc + imm : s->dnpc);
	// bltu
	INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu	 , B, s->dnpc = ((word_t)src1 < (word_t)src2) ? s->pc + imm : s->dnpc);
	// bge
	INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge	   , B, s->dnpc = ((sword_t)src1 >= (sword_t)src2) ? s->pc + imm : s->dnpc);
	// bgeu
	INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu	 , B, s->dnpc = ((word_t)src1 >= (word_t)src2) ? s->pc + imm : s->dnpc);
	// slli 
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli	 , I, R(rd) = src1 << shamt); 
	// srli 
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli	 , I, R(rd) = src1 >> shamt); 
	// srai 
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai	 , I, R(rd) = (sword_t)src1 >> shamt); 
	// lui 
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
	
	// mul
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = (word_t)src1 * (word_t)src2); 
	// mulh
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, HANDLE_MULH(src1, src2, rd)); 
	// mulhu
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, HANDLE_MULHU(src1, src2, rd)); 
	// mulhsu
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, HANDLE_MULHSU(src1, src2, rd)); 
	// div
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = (sword_t)src1 / (sword_t)src2); 
	// divu
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = (word_t)src1 / (word_t)src2); 
	// rem
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = (sword_t)src1 % (sword_t)src2); 
	// remu
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = (word_t)src1 % (word_t)src2); 

	/* control and status register */
	// csrrw
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I_CSRRW, HANDLE_CSRRW(src1, rd, imm) ); 
	
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
