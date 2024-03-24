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

wire [ADDR_WIDTH-1:0] next_pc;  // to IDU
assign next_pc = inst_jal_jalr ? nextpc_from_jal_jalr : 
															pc_reg + ADDR_WIDTH'('h4);

reg [ADDR_WIDTH-1:0] pc_reg;  
ysyx_23060208_PCreg #(.ADDR_WIDTH(ADDR_WIDTH)) PCreg_i0(
	.clk(clk),
	.rst(rst),
	.next_pc(next_pc),
	.pc(pc_reg)
);

assign valid = 1'b1;
assign pc = pc_reg;
assign addr = pc_reg;

endmodule
