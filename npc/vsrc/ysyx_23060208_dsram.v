// 本模块模拟 dsram
module ysyx_23060208_dsram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	//input rst,
	input valid, // from exu
	// srote data to mem
	input [ADDR_WIDTH-1:0] waddr,
	input [DATA_WIDTH-1:0] wdata,
	input 			 					 wen,
	input [2					 :0] store_bytes_num,
	// load data from mem
	input  [ADDR_WIDTH-1:0] raddr,
	output [DATA_WIDTH-1:0] rdata
);

import "DPI-C" function int dsram_read(input int raddr);
import "DPI-C" function void dsram_write(
	input int waddr, input int wdata, input byte wmask);

reg [DATA_WIDTH-1:0] rdata_r;
assign rdata = rdata_r;

wire [7:0] wmask;
assign wmask[7:3] = 5'b0;
assign wmask[2:0] = store_bytes_num;

always @(*) begin
	if (valid) begin  // 有读写请求时
		if (wen) 		    // 有写请求时
			dsram_write(waddr, wdata, wmask);
	end
end

always @(valid, raddr) begin
	if (valid)
		rdata_r = dsram_read(raddr);
end

endmodule
