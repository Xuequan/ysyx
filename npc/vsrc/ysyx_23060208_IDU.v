// IDU 模块
module ysyx_23060208_IDU
	#(DATA_WIDTH = 32, REG_WIDTH = 5) (
	input clk,
	input rst,
	// from IFU
	input [DATA_WIDTH-1:0]  inst,
	// from EXU 
	input [DATA_WIDTH-1:0]  wdata,
	input [REG_WIDTH-1 :0]  waddr,

	output [DATA_WIDTH-1:0] src1,
	output [DATA_WIDTH-1:0] src2,
	output [REG_WIDTH-1 :0] rd,
	output [2						:0] op
);

// 解析指令
wire [6:0] opcode;
wire [2:0] funct3;
wire [6:0] funct7;
wire [REG_WIDTH-1:0] rs1;
wire [REG_WIDTH-1:0] rs2;
wire [DATA_WIDTH-1:0] immI;
wire [DATA_WIDTH-1:0] src1_from_reg;
wire [DATA_WIDTH-1:0] src2_from_reg;
wire inst_addi; 

assign opcode = inst[6:0];
assign funct3 = inst[14:12];
assign funct7 = inst[31:25];


assign inst_addi = (opcode == 7'b001_0011) && 
									 (funct3 == 3'b111);
assign rd  = inst[11:7];
assign rs1 = inst[19:15];
assign rs2 = inst[24:20];
// I-type inst
assign immI = { {20{inst[31]}}, inst[31:20]};

assign op = 3'b000;  

ysyx_23060208_Regfile #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) regfile(
	.clk(clk),
	.wdata(wdata),
	.waddr(waddr), 
	.rdata1(src1_from_reg),
	.raddr1(rs1),
	.rdata2(src2_from_reg),
	.raddr2(rs2),
	.wen(1'b1)
);

assign src1 = src1_from_reg;
assign src2 = inst_addi ? immI : src2_from_reg;

endmodule
