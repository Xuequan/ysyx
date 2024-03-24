// IDU 模块
module ysyx_23060208_IDU
	#(DATA_WIDTH = 32, REG_WIDTH = 5) (
	input clk,
	input rst,
	// from IFU
	input [DATA_WIDTH-1:0]  inst,
	input [DATA_WIDTH-1:0]  pc_i,
	
	// from EXU 
	input [DATA_WIDTH-1:0]  wdata,
	input [REG_WIDTH-1 :0]  waddr,
	input										wen,

	// to EXU (for ALU)
	output 									inst_jal_jalr,
	output [DATA_WIDTH-1:0] pc_o,
	output [DATA_WIDTH-1:0] src1,
	output [DATA_WIDTH-1:0] src2,
	output [2						:0] op,
	output [REG_WIDTH-1 :0] rd,
	// to EXU 
		// to memory: dest = 0; to register: 1
	output 									dest, // mem or register
	output [DATA_WIDTH-1:0] to_mem_data_o,
	

	output 									inst_ebreak
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
wire [DATA_WIDTH-1:0] src1_from_reg;
wire [DATA_WIDTH-1:0] src2_from_reg;
// 判断 src2 的来源
wire 									src2_from_imm;
// 判断 src1 的来源
wire 									src1_from_pc;
wire									src1_from_zero;
// 判断inst 最终是写入 register or mem
wire									write_to_mem;
wire inst_addi; 
wire inst_auipc;
wire inst_lui;
wire inst_jal;
wire inst_jalr;
wire inst_sw;

assign opcode = inst[6:0];
assign funct3 = inst[14:12];
assign funct7 = inst[31:25];
assign rs1 = inst[19:15];
assign rs2 = inst[24:20];
assign rd  = inst[11:7];

assign inst_addi   = (opcode == 7'b001_0011) && (funct3 == 3'b0);
assign inst_ebreak = (inst == 32'b1_00000_000_00000_111_0011); 
assign inst_auipc  = (opcode == 7'b001_0111);
assign inst_lui    = (opcode == 7'b011_0111);
assign inst_jal 	 = (opcode == 7'b110_1111);
assign inst_jalr	 = (opcode == 7'b110_0111) && (funct3 == 3'b0);
assign inst_sw 		 = (opcode == 7'b010_0011) && (funct3 == 3'b010);

assign immI = { {20{inst[31]}}, inst[31:20]};
assign immJ = { {12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0 };
assign immU = { inst[31:12], 12'b0 };
assign immS = { {20{inst[31]}}, inst[31:25], inst[11:7] };


ysyx_23060208_Regfile #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) regfile(
	.clk(clk),
	.wdata(wdata),
	.waddr(waddr), 
	.rdata1(src1_from_reg),
	.raddr1(rs1),
	.rdata2(src2_from_reg),
	.raddr2(rs2),
	.wen(wen)
);

// get src1
assign src1_from_pc = inst_jal || inst_auipc;
assign src1_from_zero = inst_lui;   // 无需相加，那么将src1 = 0
assign src1 = src1_from_pc ? pc_i : 
							src1_from_zero ? 0 : src1_from_reg;

// get src2
assign src2_from_imm = inst_addi || inst_auipc || inst_lui ||
										inst_jalr || inst_jal || inst_sw;
assign imm = ({32{inst_addi  | inst_jalr}} & immI) |
						 ({32{inst_auipc | inst_lui}} & immU) |
						 ({32{inst_jal}} & immJ)							|
						 ({32{inst_sw }}  & immS);

assign src2 = src2_from_imm ? imm : src2_from_reg;

// get op
assign op = (inst_addi || inst_auipc || inst_jal || inst_jalr || 
						 inst_lui) ? 3'b000 : 3'b000;  


assign pc_o = pc_i;
assign inst_jal_jalr = inst_jal || inst_jalr;

// 判断指令最终目的
assign write_to_mem = inst_sw;
// to memory: dest = 0; to register: 1
assign dest = write_to_mem ? 0 : 1;

assign to_mem_data_o = src2_from_reg;
endmodule
