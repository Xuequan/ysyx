#include "dpi-c.h"
<<<<<<< HEAD

word_t vaddr_ifetch(vaddr_t addr, int len);
word_t vaddr_read(vaddr_t addr, int len);
void vaddr_write(vaddr_t addr, int len, word_t data);

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
	const svScope scope = svGetScopeFromName("TOP.top.idu");
=======
#include "ctrl.h"

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
  /* sdram.v 传入的地址是4字节对齐的（后两位为0）
   * 而传入的数据就是要写入的数据，并没有做处理
   * 因此，这里用 mask 合理
   */
	uint32_t waddr = (uint32_t)addr + 0xa0000000;
	int len = 0;
  uint8_t msk = (uint8_t)mask;
  int offset = 0;
  int wdata = 0;
  // sw
	if (msk == 0xf) {
    len = 4;
    offset = 0;
    wdata = data;
  } else if (msk == 0b0011) {   // sh
    len = 2;
    offset = 0;
    wdata = data & 0xffff;
  } else if (msk == 0b1100) {    // sh  
    len = 2;
    offset = 2;
    wdata = (data & 0xffff0000) >> 16;
  } else if (msk == 0b1 ) {   // sb
    len = 1;
    offset = 0;
    wdata = data & 0xff;
  } else if (msk == 0b10 ) {   // sb
    len = 1;
    offset = 1;
    wdata = (data & 0xff00) >> 8;
  } else if (msk == 0b100 ) {   // sb
    len = 1;
    offset = 2;
    wdata = (data & 0xff0000) >> 16;
  } else if (msk == 0b1000 ) {  // sb
    len = 1;
    offset = 3;
    wdata = (data & 0xff000000) >> 24;
  } else {
		printf("sdram_write(): wrong, mask is '%#x'\n", mask);
		return;
	}
#ifdef DO_PRINT
	printf("NPC: sdram_write(),initial addr = %#x, address = %#x, data = %#x, len = %d, pc = %#x\n", 
        waddr, waddr + offset, wdata, len, get_pc());
#endif
	vaddr_write(waddr + offset, len, wdata);
}

extern "C" void sdram_read(int32_t addr, int32_t *data) {    
	uint32_t raddr = (uint32_t)addr + 0xa0000000;
	*data = vaddr_read(raddr, 4);
#ifdef DO_PRINT
	printf("NPC sdram_read(): address = %#x, read data = %#x, pc = %#x\n", raddr, *data, get_pc());
#endif
}

// ------------------------------------------------------------------------------------------
// psram write and read DPI-C 
// ------------------------------------------------------------------------------------------
extern "C" void psram_read(int32_t addr, int32_t *data) {    
	*data = vaddr_read(addr + 0x80000000, 4);
#ifdef DO_PRINT_PSRAM
	printf("NPC psram_read(): address = %#x, read data = %#x, pc = %#x\n", addr + 0x80000000, *data, get_pc());
#endif
}

extern "C" void psram_write(int addr, int data, char len) {    
  // psram 是将要写入的字节挪到了开头的字节
  // 故只需要搞清楚究竟要写入几个字节就好了
	uint32_t waddr = (uint32_t)addr;
	int length = 0;
  int wdata = data;

	if      ((uint8_t)len == 0xf) { 
    length = 4;
  } else if ((uint8_t)len == 0b11) {
    length = 2;
    wdata = data & 0xffff;
  } else if ((uint8_t)len == 0b1) {
    length = 1;
    wdata = data & 0xff;
  } else {
		printf("psram_write(): wrong, len is '%#x'\n", len);
		return;
	}

#ifdef DO_PRINTPSRAM
	printf("NPC: psram_write() initial addr = %#x, address = %#x, write data = %#x, len = %d, pc = %#x\n", 
        addr, waddr + 0x80000000, wdata, length, get_pc());
#endif
	vaddr_write(waddr + 0x80000000, length, wdata);
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
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_ebreak(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void check_if_jal(svBit* o);
bool inst_is_jal() {
<<<<<<< HEAD
	const svScope scope = svGetScopeFromName("TOP.top.idu");
=======
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_jal(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void check_if_jalr(svBit* o);
bool inst_is_jalr() {
<<<<<<< HEAD
	const svScope scope = svGetScopeFromName("TOP.top.idu");
=======
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svBit a;
	check_if_jalr(&a);
	if (a == 1) return true;
	else				return false;
}

//extern void rs1_reg(svLogicVecVal* o);
uint32_t rs1(){
<<<<<<< HEAD
	const svScope scope = svGetScopeFromName("TOP.top.idu");
=======
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	rs1_reg(&o);
	return o.aval;
}
//extern void rd_reg(svLogicVecVal* o);
uint32_t rd(){
<<<<<<< HEAD
	const svScope scope = svGetScopeFromName("TOP.top.idu");
=======
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.idu");
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	rd_reg(&o);
	return o.aval;
}

//extern void get_nextPC(svLogicVecVal* o);
uint32_t nextpc(){
<<<<<<< HEAD
	const svScope scope = svGetScopeFromName("TOP.top.ifu");
=======
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.ifu");
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svLogicVecVal o;
	get_nextPC(&o);
	return o.aval;
}

<<<<<<< HEAD
uint32_t update_reg_no(){
	const svScope scope = svGetScopeFromName("TOP.top.exu");
=======
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
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svLogicVecVal no;
	update_regfile_no(&no);
	return no.aval;
}
uint32_t update_reg_data(){
<<<<<<< HEAD
	const svScope scope = svGetScopeFromName("TOP.top.exu");
=======
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
>>>>>>> tracer-ysyx
	assert(scope);
	svSetScope(scope);
	svLogicVecVal data;
	update_regfile_data(&data);
	return data.aval;
}
<<<<<<< HEAD
=======

bool check_inst_executed_already() {
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

bool check_gpio() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	gpio_check(&a);
	if (a == 1) return true;
	else				return false;
}

bool check_ps2() {
	const svScope scope = svGetScopeFromName("TOP.ysyxSoCFull.asic.cpu.cpu.exu");
	assert(scope);
	svSetScope(scope);
	svBit a;
	ps2_check(&a);
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
>>>>>>> tracer-ysyx
