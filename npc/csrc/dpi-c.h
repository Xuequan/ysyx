#include "memory.h"
#include <cstdint>
#include "Vtop__Dpi.h"

word_t vaddr_ifetch(vaddr_t addr, int len);
// 总是读取地址为 raddr & ~0x3u 的4字节返回
// raddr 是 vaddr                                                               
extern "C" int isram_read(int raddr);
  // 总是往地址为 'waddr & ~0x3u '的4字节按写掩码 'wmask' 写入 'wdata'
  // 'wmask' 中每比特表示 'wdata' 中1个字节的掩码，
  // 如 'wmask = 0x3' 代表只写入最低2个字节，内存中的其它字节保持不>变
extern "C" void dsram_write(int waddr, int wdata, char wmask);

// below from ysyx_23060208_IDU
extern void check_if_ebreak(svBit* o);
bool inst_is_ebreak();

extern void check_if_jal(svBit* o);
bool inst_is_jal();

extern void check_if_jalr(svBit* o);
bool inst_is_jalr();

extern void rs1_reg(svLogicVecVal* o);
uint32_t rs1();

extern void rd_reg(svLogicVecVal* o);
uint32_t rd();

// below from ysyx_23060208_IFU
extern void nectPC(svLogicVecVal* o);
uint32_t nextpc();

// 给出下个周期要更新的 regfile
extern void update_regfile_no(svLogicVecVal* reg_no);
uint32_t update_reg_no();

extern void update_regfile_data(svLogicVecVal* din);
uint32_t update_reg_data();
