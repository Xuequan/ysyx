// 本模块模拟 inst sram, 输入地址，得到指令
module ysyx_2306208_isram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [ADDR_WIDTH-1:0] addr,
	output reg [DATA_WIDTH-1:0] inst_o
);

import "DPI-C" function int pmem_read(input int raddr);
import "DPC-C" function void pmem_write(
	input int waddr, input int wdata, input byte wmask);

reg [DATA_WIDTH-1:0] rdata;
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

endmodule
