#include "dpi-c.h"

word_t vaddr_ifetch(vaddr_t addr, int len);
word_t vaddr_read(vaddr_t addr, int len);

void vaddr_write(vaddr_t addr, int len, word_t data);

extern "C" void flash_read(int32_t addr, int32_t *data) {    
	*data = vaddr_read(addr + 0x30000000, 4);
}
extern "C" void mrom_read(int32_t addr, int32_t *data) {
	//*data = 0x00100073; // ebreak inst
	//int32_t ret = vaddr_read(addr, 4);
	*data = vaddr_read(addr, 4);
	//printf("mrom_read(): addr = %#x, return %#x\n", addr, *data);
}

/* 总是读取地址为 raddr & ~0x3u 的4字节返回  */
extern "C" int isram_read(int raddr) {
  return vaddr_ifetch(raddr, 4);
}

extern "C" int dsram_read(int raddr) {
  return vaddr_read(raddr, 4);
}

extern "C" void dsram_write(int waddr, int wdata, char wmask) {
	uint32_t addr = (uint32_t)waddr;
	if ((uint8_t)wmask == 1 )
  	return vaddr_write(addr, 1, wdata);
	else if ((uint8_t)wmask == 2) 
  	return vaddr_write(addr, 2, wdata);
	else if ((uint8_t)wmask == 4) 
  	return vaddr_write(addr, 4, wdata);
	else {
		printf("Error, wmask = %#x error\n", (uint8_t)wmask);
		return;
	}
}

//extern void check_if_ebreak(svBit* o);
bool inst_is_ebreak() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_ebreak(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void check_if_jal(svBit* o);
bool inst_is_jal() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_jal(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void check_if_jalr(svBit* o);
bool inst_is_jalr() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_jalr(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void rs1_reg(svLogicVecVal* o);
uint32_t rs1(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	rs1_reg(&o);
	return o.aval;
}
//extern void rd_reg(svLogicVecVal* o);
uint32_t rd(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	rd_reg(&o);
	return o.aval;
}

//extern void get_nextPC(svLogicVecVal* o);
uint32_t nextpc(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.ifu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	get_nextPC(&o);
	return o.aval;
}

uint32_t get_pc(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.ifu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	get_PC(&o);
	return o.aval;
}

uint32_t get_inst(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	get_inst_from_exu(&o);
	return o.aval;
}

uint32_t update_reg_no(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal no;
	update_regfile_no(&no);
	return no.aval;
}
uint32_t update_reg_data(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal data;
	update_regfile_data(&data);
	return data.aval;
}

bool check_exu_ready_go() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	exu_will_go_next_clock_signal(&a);
	if (a == 1) return true;
	else				return false;
}
bool check_ifu_ready_go() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.ifu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	ifu_ready_go_signal(&a);
	if (a == 1) return true;
	else				return false;
}
bool check_clint_read() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	clint_addr_check(&a);
	if (a == 1) return true;
	else				return false;
}
bool check_uart_write() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	uart_write_check(&a);
	if (a == 1) return true;
	else				return false;
}
bool check_uart_read() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	uart_read_check(&a);
	if (a == 1) return true;
	else				return false;
}

bool check_spi_master_write() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	spi_master_write_check(&a);
	if (a == 1) return true;
	else				return false;
}
bool check_spi_master_read() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	spi_master_read_check(&a);
	if (a == 1) return true;
	else				return false;
}
bool check_access_fault_exu() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_access_fault(&a);
	if (a == 1) return true;
	else				return false;
}
bool check_access_fault_ifu() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.ifu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_access_fault_ifu(&a);
	/*
	if (a == 1) return true;
	else				return false;
	*/
	return false;
}
