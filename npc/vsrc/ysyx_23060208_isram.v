// 本模块模拟 isram
module ysyx_23060208_isram
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,

	input 									ifu_allowin,
	input [DATA_WIDTH-1:0]  isram_raddr,
	output								  isram_ready,
	output [DATA_WIDTH-1:0] isram_rdata
);

import "DPI-C" function int isram_read(input int isram_raddr);

reg [DATA_WIDTH-1:0] rdata;

always @(posedge clk) begin
	if (rst) begin
		rdata <= 0;
	end 
	else if (ifu_allowin) begin  // 有读写请求时
		rdata <= isram_read(isram_raddr);
	end
	else begin
		rdata <= 0;
	end
end

assign isram_rdata = rdata;
assign isram_ready = 1'b1;
endmodule
