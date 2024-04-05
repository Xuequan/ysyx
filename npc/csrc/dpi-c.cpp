/*
#include "memory.h"
#include <cstdint>
#include "Vtop__Dpi.h"
*/
#include "dpi-c.h"

word_t vaddr_ifetch(vaddr_t addr, int len);
// 总是读取地址为 raddr & ~0x3u 的4字节返回
// raddr 是 vaddr                                                               
extern "C" int isram_read(int raddr) {
  return vaddr_ifetch(raddr, 4);
}
  // 总是往地址为 'waddr & ~0x3u '的4字节按写掩码 'wmask' 写入 'wdata'
  // 'wmask' 中每比特表示 'wdata' 中1个字节的掩码，
  // 如 'wmask = 0x3' 代表只写入最低2个字节，内存中的其它字节保持不>变
extern "C" void dsram_write(int waddr, int wdata, char wmask) {
  return;
}

//extern void check_if_ebreak(svBit* o);
bool inst_is_ebreak() {
	svBit a;
	check_if_ebreak(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void check_if_jal(svBit* o);
bool inst_is_jal() {
	svBit a;
	check_if_jal(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void check_if_jalr(svBit* o);
bool inst_is_jalr() {
	svBit a;
	check_if_jalr(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void rs1_reg(svLogicVecVal* o);
uint32_t rs1(){
	svLogicVecVal o;
	rs1_reg(&o);
	return o.aval;
}
//extern void rd_reg(svLogicVecVal* o);
uint32_t rd(){
	svLogicVecVal o;
	rd_reg(&o);
	return o.aval;
}

//extern void nextPC(svLogicVecVal* o);
uint32_t nextpc(){
	svLogicVecVal o;
	nextPC(&o);
	return o.aval;
}
