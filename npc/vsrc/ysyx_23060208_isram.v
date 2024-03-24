// 本模块模拟 sram, 输入地址，得到指令
module ysyx_23060208_isram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input valid,
	input [ADDR_WIDTH-1:0] raddr,
	output reg [DATA_WIDTH-1:0] inst_o
);

import "DPI-C" function int pmem_read(input int raddr);
/*
import "DPI-C" function void pmem_write(
	input int waddr, input int wdata, input byte wmask);
*/

reg [DATA_WIDTH-1:0] rdata;
assign inst_o = rdata;

always @(posedge clk) begin
	if (rst) begin
		rdata <= 0;
	end 
	else if (valid) begin  // 有读写请求时
		rdata <= pmem_read(raddr);
	end
	else begin
		rdata <= 0;
	end
end

/*
always @(*) begin
	if (valid) begin  // 有读写请求时
		rdata = pmem_read(raddr);
		if (wen) begin begin  // 有写请求时
			pmem_write(waddr, wdata, wmask);
		end
	end
	else begin
		rdata = 0;
	end
end
*/

endmodule
