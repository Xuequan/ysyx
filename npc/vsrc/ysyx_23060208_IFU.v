// IFU 模块
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [DATA_WIDTH-1:0] inst_i,  // from inst ram

	output [ADDR_WIDTH-1:0] addr_o // to inst ram
);

wire [ADDR_WIDTH-1:0] dnpc;  // to IDU
//assign dnpc = addr_o + ADDR_WIDTH'h4;
assign dnpc = addr_o + ADDR_WIDTH'('h4);

ysyx_23060208_PCreg #(.ADDR_WIDTH(ADDR_WIDTH)) i0(
	.clk(clk),
	.rst(rst),
	.dnpc(dnpc),
	.pc(addr_o)
);

endmodule

	
