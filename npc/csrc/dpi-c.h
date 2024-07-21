#ifndef __DPIC_H__
#define __DPIC_H__

#include "memory.h"
#include <cstdint>
#include "VysyxSoCFull__Dpi.h"

extern "C" void psram_read(int32_t addr, int32_t *data); 
extern "C" void psram_write(int addr, int data); 

extern "C" void flash_read(int32_t addr, int32_t *data); 
extern "C" void mrom_read(int32_t addr, int32_t *data); 

// below from ysyxSoCFull_IDU
//extern void check_if_ebreak(svBit* o);
bool inst_is_ebreak();

//extern void check_if_jal(svBit* o);
bool inst_is_jal();

//extern void check_if_jalr(svBit* o);
bool inst_is_jalr();

//extern void rs1_reg(svLogicVecVal* o);
uint32_t rs1();

//extern void rd_reg(svLogicVecVal* o);
uint32_t rd();

// below from ysyxSoCFull_IFU
//extern void get_nectPC(svLogicVecVal* o);
uint32_t nextpc();

// 给出下个周期要更新的 regfile
//extern void update_regfile_no(svLogicVecVal* reg_no);
uint32_t update_reg_no();

//extern void update_regfile_data(svLogicVecVal* din);
uint32_t update_reg_data();

bool check_exu_ready_go();

uint32_t get_pc();

uint32_t get_inst();

bool check_ifu_ready_go();

bool check_clint_read();

bool check_uart_write();
bool check_uart_read();

bool check_spi_master_write();
bool check_spi_master_read();

bool check_access_fault_ifu();
bool check_access_fault_exu();

#endif
