// pc register
// wen 始终有效
module ysyx_23060208_PCreg
	#(ADDR_WIDTH = 32) (
	input clk,
	input rst,
	//input wen,
	// next_pc
	input  [ADDR_WIDTH-1:0] next_pc,
	output reg [ADDR_WIDTH-1:0] pc
);

Reg #(.WIDTH(ADDR_WIDTH), .RESET_VAL(ADDR_WIDTH'('h8000_0000))) reg_i0(
//Reg #(.WIDTH(ADDR_WIDTH), .RESET_VAL(ADDR_WIDTH'('h7fff_fffc))) reg_i0(
// Reg #(.WIDTH(ADDR_WIDTH), .RESET_VAL(32'h8000_0000)) reg_i0(
	.clk(clk),
	.rst(rst),
	.din(next_pc),
	.dout(pc),
	.wen(1'b1)   
);

endmodule
