// EXU 模块
module ysyx_23060208_EXU
	#(DATA_WIDTH = 32, REG_WIDTH = 5) (
	input clk,
	input rst,

	input 								 inst_jal_jalr,
	input [DATA_WIDTH-1:0] pc,
	input [DATA_WIDTH-1:0] src1,
	input [DATA_WIDTH-1:0] src2,
	input [REG_WIDTH-1:0]  rd,
	input [2					 :0] op,
	// to IFU for gen nextPC
	output [DATA_WIDTH-1:0] nextpc_from_jal_jalr,
	// output to register file
	output [REG_WIDTH-1:0] waddr,
	output [DATA_WIDTH-1:0] wdata
);

wire [DATA_WIDTH-1:0] alu_result;
wire overflow;

ysyx_23060208_alu #(.DATA_WIDTH(DATA_WIDTH)) i0(
	.result(alu_result),
	.overflow(overflow),
	.op(op),
	.src1(src1),
	.src2(src2)
);

// 若是 jal, jalr, 那么将 rd <- pc + 4
assign wdata = inst_jal_jalr ? pc + 4 : alu_result;
assign waddr = rd;

assign nextpc_from_jal_jalr = alu_result;

endmodule

