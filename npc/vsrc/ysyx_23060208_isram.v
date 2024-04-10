// 本模块模拟 isram
module ysyx_23060208_isram
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,
	input valid,
	input [DATA_WIDTH-1:0] nextPC,
	output reg [DATA_WIDTH-1:0] inst_o
);

import "DPI-C" function int isram_read(input int nextPC);

reg [DATA_WIDTH-1:0] rdata;
assign inst_o = rdata;

always @(posedge clk) begin
	if (rst) begin
		rdata <= 0;
	end 
	else if (valid) begin  // 有读写请求时
		rdata <= isram_read(nextPC);
	end
	else begin
		rdata <= 0;
	end
end

endmodule
