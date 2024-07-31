#include "dpi-c.h"

extern word_t vaddr_read(vaddr_t addr, int len);
extern void vaddr_write(vaddr_t addr, int len, word_t data);

uint32_t get_pc(){
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.ifu");
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	get_PC(&o);
	return o.aval;
}

// ------------------------------------------------------------------------------------------
// sdram write and read DPI-C 
// ------------------------------------------------------------------------------------------
extern "C" void sdram_write(int addr, int data, char mask) {    
	uint32_t waddr = (uint32_t)addr + 0xa0000000;
	int len = 0;
  uint8_t msk = (uint8_t)mask;
  int offset = 0;
	if (msk == 0xf) {
    len = 4;
    offset = 0;
  } else if (msk == 0b0011) {
    len = 2;
    offset = 0;
  } else if (msk == 0b1100) {
    len = 2;
    offset = 2;
  } else if (msk == 0b1 ) {
    len = 1;
    offset = 0;
  } else if (msk == 0b10 ) {
    len = 1;
    offset = 1;
  } else if (msk == 0b100 ) {
    len = 1;
    offset = 2;
  } else if (msk == 0b1000 ) {
    len = 1;
    offset = 3;
  } else {
		printf("sdram_write(): wrong, mask is '%#x'\n", mask);
		return;
	}
	printf("NPC: sdram write address = %#x, data = %#x, len = %d, pc = %#x\n", waddr + offset, data, len, get_pc());
	vaddr_write(waddr + offset, len, data);
}

extern "C" void sdram_read(int32_t addr, int32_t *data) {    
	uint32_t raddr = (uint32_t)addr + 0xa0000000;
	*data = vaddr_read(raddr, 4);
	printf("NPC sdram_read(): address = %#x, read data = %#x, pc = %#x\n", raddr, *data, get_pc());
}

// ------------------------------------------------------------------------------------------
// psram write and read DPI-C 
// ------------------------------------------------------------------------------------------
extern "C" void psram_read(int32_t addr, int32_t *data) {    
	*data = vaddr_read(addr + 0x80000000, 4);
	//printf("NPC psram_read(): address = %#x, read data = %#x, pc = %#x\n", addr + 0x80000000, *data, get_pc());
}

extern "C" void psram_write(int addr, int data, char mask) {    
	uint32_t waddr = (uint32_t)addr;
	int len = 0;
	if ((uint8_t)mask == 0xff) len = 4;
	else if ((uint8_t)mask == 0b1111) len = 2;
	else if ((uint8_t)mask == 0b11) len = 1;
	else {
		printf("psram_write(): wrong, mask is '%#x'\n", mask);
		return;
	}
	//printf("NPC: write address = %#x, write data = %#x, len = %d, pc = %#x\n", waddr + 0x80000000, data, len, get_pc());
	vaddr_write(waddr + 0x80000000, len, data);
}

// ------------------------------------------------------------------------------------------
// flash write and read DPI-C 
// ------------------------------------------------------------------------------------------
extern "C" void flash_read(int32_t addr, int32_t *data) {    
	*data = vaddr_read(addr + 0x30000000, 4);
}
extern "C" void mrom_read(int32_t addr, int32_t *data) {
	*data = vaddr_read(addr, 4);
}

// ------------------------------------------------------------------------------------------
// some check DPI-C
// ------------------------------------------------------------------------------------------
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
	if (a == 1) return true;
	else				return false;
}
