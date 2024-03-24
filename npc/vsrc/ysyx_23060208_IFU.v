// IFU 模块
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [DATA_WIDTH-1:0]  inst_i,  // from inst ram

	// from IDU
	input 								  inst_jal_jalr, 
	// from EXU
	input [DATA_WIDTH-1:0]  nextpc_from_jal_jalr,

	output reg 							valid,						
	output [DATA_WIDTH-1:0] pc,  // to IDU
	output [ADDR_WIDTH-1:0] addr    // to inst ram
);

//reg [ADDR_WIDTH-1:0] next_pc; 
//assign next_pc = inst_jal_jalr ? nextpc_from_jal_jalr :
assign addr = inst_jal_jalr ? nextpc_from_jal_jalr :
													pc + 4;

ysyx_23060208_PC #(.ADDR_WIDTH(ADDR_WIDTH)) PC_i0(
	.clk(clk),
	.rst(rst),
	.wen(1'b1),
	.next_pc(addr),
	.pc(pc)
);

assign valid = 1'b1;
//assign addr = next_pc;
endmodule
