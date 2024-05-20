// IDU 模块
module ysyx_23060208_IDU
	#(DATA_WIDTH = 32, REG_WIDTH = 5) (
	input clk,
	input rst,
	// from IFU
	input [DATA_WIDTH-1:0]  inst,
	input [DATA_WIDTH-1:0]  pc_i,
	
	// from EXU 
	input [DATA_WIDTH-1:0]  regfile_wdata,
	input [REG_WIDTH-1 :0]  regfile_waddr,
	input										regfile_wen,

	// to EXU (for ALU)
	output [DATA_WIDTH-1:0] pc_o,
	output [DATA_WIDTH-1:0] src1,
	output [DATA_WIDTH-1:0] src2,
	output [17					:0] op,
	output [REG_WIDTH-1 :0] rd,

	// instruction write to regfile or memory or no both
	// to memory: regfile_mem_mux = 2'b10; to register: 2'b01
	output [1						:0]	regfile_mem_mux, 
	// unconditional jump (jal & jalr)
	output [1           :0]	uncond_jump_inst,
	// conditional branch (to EXU)
	output									cond_branch_inst,
	output [DATA_WIDTH-1:0] cond_branch_target,
	// load instruction
	output [4						:0] load_inst,
	// store instruction write to memory data
	output [2						:0] store_inst,
	output [DATA_WIDTH-1:0] store_data_raw,

	// CSR for nextpc
	output [DATA_WIDTH-1:0] csr_nextpc,
	output									csr_nextpc_taken,
	
	// from EXU, CSR write data 
	input [DATA_WIDTH-1:0] csr_wdata,
	// to EXU
	output [1:0] 					 csr_inst
);

// 解析指令
wire [6:0] opcode;
wire [2:0] funct3;
wire [6:0] funct7;
wire [REG_WIDTH-1:0] rs1;
wire [REG_WIDTH-1:0] rs2;
wire [DATA_WIDTH-1:0] imm;
wire [DATA_WIDTH-1:0] immI;
wire [DATA_WIDTH-1:0] immU;
wire [DATA_WIDTH-1:0] immJ;
wire [DATA_WIDTH-1:0] immS;
wire [DATA_WIDTH-1:0] immB;
wire [DATA_WIDTH-1:0] src1_from_reg;
wire [DATA_WIDTH-1:0] src2_from_reg;
wire [DATA_WIDTH-1:0] csr_rdata;  // put into src2

assign opcode = inst[6:0];
assign funct3 = inst[14:12];
assign funct7 = inst[31:25];
assign rs1 = inst[19:15];
assign rs2 = inst[24:20];
assign rd  = inst[11:7];
assign csr = inst[31:20]; 
/* ================ 判断alu src1, src2 来源 ========= */
// 判断 src2 的来源
wire 									src2_from_imm;
wire									src2_from_csr;
// 判断 src1 的来源
wire 									src1_from_pc;
   // src1_is_zero 和 src1_is_none 可以弄成一个
wire									src1_is_zero; // like csrrw, no need src1 for ALU
wire									src1_is_none; // like lui, no need src1

/*========== 判断inst 最终是写入 register or mem ====== */
wire									write_to_mem;

/* ==================inst_instruction ================*/
// integer register-immediate instructions
wire inst_addi; 
wire inst_slti;
wire inst_sltiu;
wire inst_andi; 
wire inst_ori; 
wire inst_xori; 

wire inst_slli;
wire inst_srli;
wire inst_srai;

wire inst_lui;
wire inst_auipc;

// integer register-register instructions
wire inst_add;
wire inst_slt;
wire inst_sltu;
wire inst_and;
wire inst_or;
wire inst_xor;
wire inst_sll;
wire inst_srl;
wire inst_sub;
wire inst_sra;

// NOP instruction
//wire inst_nop;
//assign inst_nop   = (opcode == 7'b001_0011) && (funct3 == 3'b0);

// unconditional jump
wire inst_jal;
wire inst_jalr;

// conditional branches
wire inst_beq;
wire inst_bne;
wire inst_blt;
wire inst_bltu;
wire inst_bge;
wire inst_bgeu;

// load instructions
wire inst_lw;
wire inst_lb;
wire inst_lh;
wire inst_lbu;
wire inst_lhu;

// store instructions
wire inst_sw;
wire inst_sb;
wire inst_sh;

// environment call and breakpoints
wire inst_ecall;
wire inst_ebreak;

// Machine-mode privileged instructions
wire inst_mret;
// 'Zicsr', CSR instructions
wire inst_csrrw;   // rd <-csr;  csr <- src1

			// 利用 alu 来计算csr <- src1 | csr
wire inst_csrrs;   // rd <- csr; csr <- src1 | csr

// integer register-immediate instructions
assign inst_addi   = (opcode == 7'b001_0011) && (funct3 == 3'b0);
assign inst_slti   = (opcode == 7'b001_0011) && (funct3 == 3'b010);
assign inst_sltiu  = (opcode == 7'b001_0011) && (funct3 == 3'b011);
assign inst_andi   = (opcode == 7'b001_0011) && (funct3 == 3'b111);
assign inst_ori    = (opcode == 7'b001_0011) && (funct3 == 3'b110);
assign inst_xori   = (opcode == 7'b001_0011) && (funct3 == 3'b100);

assign inst_slli   = (opcode == 7'b001_0011) && (funct3 == 3'b001);
assign inst_srli   = (opcode == 7'b001_0011) && (funct3 == 3'b101)
									 && (inst[30] == 1'b0);
assign inst_srai   = (opcode == 7'b001_0011) && (funct3 == 3'b101)
									 && (inst[30] == 1'b1);

assign inst_lui    = (opcode == 7'b011_0111);
assign inst_auipc  = (opcode == 7'b001_0111);

// integer register-register instructions
assign inst_add    = (opcode == 7'b011_0011) && (funct3 == 3'b000)
									&& (funct7 == 7'b000_0000);
assign inst_slt    = (opcode == 7'b011_0011) && (funct3 == 3'b010)
									&& (funct7 == 7'b000_0000);
assign inst_sltu   = (opcode == 7'b011_0011) && (funct3 == 3'b011)
									&& (funct7 == 7'b000_0000);
assign inst_and    = (opcode == 7'b011_0011) && (funct3 == 3'b111)
									&& (funct7 == 7'b000_0000);
assign inst_or    = (opcode == 7'b011_0011) && (funct3 == 3'b110)
									&& (funct7 == 7'b000_0000);
assign inst_xor    = (opcode == 7'b011_0011) && (funct3 == 3'b100)
									&& (funct7 == 7'b000_0000);
assign inst_sll    = (opcode == 7'b011_0011) && (funct3 == 3'b001)
									&& (funct7 == 7'b000_0000);
assign inst_srl    = (opcode == 7'b011_0011) && (funct3 == 3'b101)
									&& (funct7 == 7'b000_0000);
assign inst_sub    = (opcode == 7'b011_0011) && (funct3 == 3'b000)
									&& (funct7 == 7'b010_0000);
assign inst_sra    = (opcode == 7'b011_0011) && (funct3 == 3'b101)
									&& (funct7 == 7'b010_0000);

// unconditional jump
assign inst_jal 	 = (opcode == 7'b110_1111);
assign inst_jalr	 = (opcode == 7'b110_0111) && (funct3 == 3'b0);

// conditional branches
assign inst_beq	 = (opcode == 7'b110_0011) && (funct3 == 3'b000);
assign inst_bne	 = (opcode == 7'b110_0011) && (funct3 == 3'b001);
assign inst_blt	 = (opcode == 7'b110_0011) && (funct3 == 3'b100);
assign inst_bltu = (opcode == 7'b110_0011) && (funct3 == 3'b110);
assign inst_bge	 = (opcode == 7'b110_0011) && (funct3 == 3'b101);
assign inst_bgeu = (opcode == 7'b110_0011) && (funct3 == 3'b111);

// load instructions
assign inst_lw = (opcode == 7'b000_0011) && (funct3 == 3'b010);
assign inst_lb = (opcode == 7'b000_0011) && (funct3 == 3'b000);
assign inst_lh = (opcode == 7'b000_0011) && (funct3 == 3'b001);
assign inst_lbu= (opcode == 7'b000_0011) && (funct3 == 3'b100);
assign inst_lhu= (opcode == 7'b000_0011) && (funct3 == 3'b101);
// store instructions
assign inst_sw  = (opcode == 7'b010_0011) && (funct3 == 3'b010);
assign inst_sb  = (opcode == 7'b010_0011) && (funct3 == 3'b000);
assign inst_sh  = (opcode == 7'b010_0011) && (funct3 == 3'b001);

// environment call and breakpoints
assign inst_ecall  = (inst == 32'b0_00000_000_00000_111_0011); 
assign inst_ebreak = (inst == 32'b1_00000_000_00000_111_0011); 

// Machine-mode privileged instructions
assign inst_mret   = (inst == 32'b0011000_00010_00000_000_00000_1110011); 

// 'Zicsr', CSR instructions
assign inst_csrrw = (opcode == 7'b111_0011) && (funct3 == 3'b001);
assign inst_csrrs = (opcode == 7'b111_0011) && (funct3 == 3'b010);

/*============== 不同类型指令对应的 Imm ============ */
assign immI = { {20{inst[31]}}, inst[31:20]};
assign immJ = { {12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0 };
assign immU = { inst[31:12], 12'b0 };
assign immS = { {20{inst[31]}}, inst[31:25], inst[11:7] };
assign immB = { {20{inst[31]}}, inst[7], inst[30:25], inst[11:8], 1'b0 };


ysyx_23060208_regfile #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) regfile(
	.clk(clk),
	.rst(rst),
	.wdata(regfile_wdata),
	.waddr(regfile_waddr), 
	.rdata1(src1_from_reg),
	.raddr1(rs1),
	.rdata2(src2_from_reg),
	.raddr2(rs2),
	.wen(regfile_wen)
);

/* ================ get alu src1 ====================== */
assign src1_from_pc = inst_jal || inst_auipc;
assign src1_is_zero = 0;
assign src1_is_none = inst_lui;
assign src1 = src1_from_pc ? pc_i : 
							src1_is_zero ? 0 : 
							src1_is_none ? 0 : 
														src1_from_reg;

/* ================ get alu src2 ====================== */
wire imm_is_Itype;
wire imm_is_Utype;
wire imm_is_Jtype;
wire imm_is_Stype;

assign imm_is_Itype = inst_addi | inst_slti | inst_sltiu 
								| inst_ori | inst_andi | inst_xori
								| inst_slli | inst_srli | inst_srai
								| inst_jalr
								| inst_lw | inst_lb | inst_lh 
								| inst_lhu | inst_lbu;
assign imm_is_Utype = inst_lui  | inst_auipc;
assign imm_is_Jtype = inst_jal;
assign imm_is_Stype = inst_sw | inst_sh | inst_sb;

assign src2_from_imm = imm_is_Itype || imm_is_Utype
										|| imm_is_Jtype 
										|| imm_is_Stype;

assign src2_from_csr = inst_csrrw | inst_csrrs;

assign imm = ( {32{imm_is_Itype}} & immI) |
						 ( {32{imm_is_Utype}} & immU) |
						 ( {32{imm_is_Jtype}} & immJ)	|
						 ( {32{imm_is_Stype}} & immS);

assign src2 = src2_from_imm ? imm 
						: src2_from_csr ? csr_rdata 
						: src2_from_reg;

/* ================ get op ====================== */
wire op_add;   //加法操作
wire op_sub;   //减法操作
wire op_slt;   //有符号比较，小于置位
wire op_sltu;  //无符号比较，小于置位
wire op_and;   //按位与
wire op_nor;   //按位或非
wire op_or;    //按位或
wire op_xor;   //按位异或
wire op_sll;   //逻辑左移
wire op_srl;   //逻辑右移
wire op_sra;   //算术右移
wire op_lui;   //立即数置于高半部分
wire op_beq;
wire op_bne;
wire op_blt;
wire op_bltu;
wire op_bge;   
wire op_bgeu;

assign op_add = inst_addi | inst_auipc 
							| inst_add  | inst_jal 
							| inst_jalr
							| inst_lw   | inst_lh
							| inst_lhu  | inst_lb
							| inst_lbu  
							| inst_sw   | inst_sh
							| inst_sb;
assign op_sub = inst_sub;
assign op_slt = inst_slti | inst_slt;
assign op_sltu = inst_sltiu | inst_sltu;
assign op_and = inst_andi | inst_and;
assign op_or = inst_ori | inst_or 
							| inst_csrrs;
assign op_xor = inst_xori | inst_xor;
assign op_sll = inst_slli | inst_sll;
assign op_srl = inst_srli | inst_srl;
assign op_sra = inst_srai | inst_sra;
assign op_lui = inst_lui;
assign op_beq = inst_beq;
assign op_bne = inst_bne;
assign op_blt = inst_blt;
assign op_bltu= inst_bltu;
assign op_bge = inst_bge;
assign op_bgeu= inst_bgeu;

assign op[ 0] = op_add;
assign op[ 1] = op_sub;
assign op[ 2] = op_slt;
assign op[ 3] = op_sltu;
assign op[ 4] = op_and;
assign op[ 5] = op_nor;
assign op[ 6] = op_or;
assign op[ 7] = op_xor;
assign op[ 8] = op_sll;
assign op[ 9] = op_srl;
assign op[10] = op_sra;
assign op[11] = op_lui;
assign op[12] = op_beq;
assign op[13] = op_bne;
assign op[14] = op_blt;
assign op[15] = op_bltu;
assign op[16] = op_bge;
assign op[17] = op_bgeu;

/* ================ CSRs ====================== */
wire mtvec_wen;
//wire [DATA_WIDTH-1:0] mtvec_wdata;
wire [DATA_WIDTH-1:0] mtvec_rdata;
/*
assign mtvec_wen = inst_ecall;
assign mtvec_wdata = pc_i;
*/

ysyx_23060208_mtvec #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mtvec(
	.clk(clk),
	.rst(rst),
	.wdata(csr_wdata),
	.rdata(mtvec_rdata),
	.wen(mtvec_wen)
);

wire mcause_wen;
wire [DATA_WIDTH-1:0] mcause_rdata;
wire [DATA_WIDTH-1:0] mcause_wdata;
assign mcause_wdata = inst_ecall ? 32'hb : csr_wdata;

ysyx_23060208_mcause #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mcause(
	.clk(clk),
	.rst(rst),
	.wdata(mcause_wdata),
	.rdata(mcause_rdata),
	.wen(mcause_wen)
);

wire mepc_wen;
wire [DATA_WIDTH-1:0] mepc_rdata;
wire [DATA_WIDTH-1:0] mepc_wdata;
assign mepc_wdata = inst_ecall ? pc_i : csr_wdata;

ysyx_23060208_mepc #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mepc(
	.clk(clk),
	.rst(rst),
	.wdata(mepc_wdata),
	.rdata(mepc_rdata),
	.wen(mepc_wen)
);
wire mstatus_wen;
wire [DATA_WIDTH-1:0] mstatus_rdata;
//wire [DATA_WIDTH-1:0] mstatus_wdata;
ysyx_23060208_mstatus #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mstatus(
	.clk(clk),
	.rst(rst),
	.wdata(csr_wdata),
	.rdata(mstatus_rdata),
	.wen(mstatus_wen)
);

/* ================ write to CSRs ============= */
assign mtvec_wen = (csr == 12'h305 && (inst_csrrw | inst_csrrs)) ;
assign mepc_wen  = (csr == 12'h341 && (inst_csrrw | inst_csrrs)) || inst_ecall;
assign mcause_wen = (csr == 12'h342 && (inst_csrrw | inst_csrrs)) || inst_ecall;
assign mstatus_wen = (csr == 12'h300 && (inst_csrrw | inst_csrrs));  

assign csr_rdata =  ({DATA_WIDTH{csr == 12'h305}} & mtvec_rdata) 
									| ({DATA_WIDTH{csr == 12'h341}} & mepc_rdata)
									| ({DATA_WIDTH{csr == 12'h342}} & mcause_rdata)
									| ({DATA_WIDTH{csr == 12'h300}} & mstatus_rdata);

/* ================= assign output ============= */
assign pc_o = pc_i;

assign csr_nextpc = ({DATA_WIDTH{inst_ecall}} & mtvec_rdata) 
									| ({DATA_WIDTH{inst_mret }} & mepc_rdata );
assign csr_nextpc_taken = inst_ecall | inst_mret;

// 判断指令最终目的: write to regfile or store to memory
assign write_to_mem = inst_sw | inst_sh | inst_sb;
assign regfile_mem_mux[1] = write_to_mem;
assign regfile_mem_mux[0] = ~(write_to_mem | cond_branch_inst);

assign store_data_raw = src2_from_reg;
assign store_inst[0] = inst_sw;
assign store_inst[1] = inst_sh;
assign store_inst[2] = inst_sb;

// unconditional jump
assign uncond_jump_inst[0] = inst_jal;
assign uncond_jump_inst[1] = inst_jalr;

// conditional branch 是否跳转由比较 src1, src2 两寄存器决定
// 该部分交给 alu
// 跳转地址 B + pc_i
assign cond_branch_target = immB + pc_i;
assign cond_branch_inst = inst_beq | inst_bne
										| inst_blt | inst_bltu
										| inst_bge | inst_bgeu;

// load instructions
assign load_inst[0] = inst_lw;
assign load_inst[1] = inst_lh;
assign load_inst[2] = inst_lhu;
assign load_inst[3] = inst_lb;
assign load_inst[4] = inst_lbu;

assign csr_inst[0] = inst_csrrw;
assign csr_inst[1] = inst_csrrs;
//======================== DPI-C ================================
export "DPI-C" task check_if_ebreak;
task check_if_ebreak (output bit o);
	o = inst_ebreak;
endtask

export "DPI-C" task check_if_jal;
task check_if_jal (output bit o);
	o = inst_jal;
endtask

export "DPI-C" task check_if_jalr;
task check_if_jalr (output bit o);
	o = inst_jalr;
endtask

export "DPI-C" task rs1_reg;
task rs1_reg (output [4:0] o);
	o = rs1;
endtask
export "DPI-C" task rd_reg;
task rd_reg (output [4:0] o);
	o = rd;
endtask
endmodule
