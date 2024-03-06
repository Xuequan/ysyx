// IFU 模块
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [DATA_WIDTH-1:0] inst_i,  // from inst ram

	//output reg [ADDR_WIDTH-1:0] addr_o // to inst ram
	output [ADDR_WIDTH-1:0] addr
);

wire [ADDR_WIDTH-1:0] next_pc;  // to IDU
wire [ADDR_WIDTH-1:0] addr_o;
assign next_pc = addr_o + ADDR_WIDTH'('h4);

ysyx_23060208_PCreg #(.ADDR_WIDTH(ADDR_WIDTH)) PCreg_i0(
	.clk(clk),
	.rst(rst),
	.next_pc(next_pc),
	.pc(addr_o)
);

reg [ADDR_WIDTH-1:0] addr_r; 
always @(posedge clk) begin
		addr_r <= addr_o;
end
assign addr = addr_r;

endmodule
