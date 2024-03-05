// register file
module ysyx_23060208_Regfile 
	// REG_WIDTH: regfile address width
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
	input clk,
	input [DATA_WIDTH-1:0] wdata,
	input [REG_WIDTH-1:0] waddr,

	output [DATA_WIDTH-1:0] rdata1,
	input  [REG_WIDTH-1:0] raddr1,

	output [DATA_WIDTH-1:0] rdata2,
	input  [REG_WIDTH-1:0] raddr2,
	input wen
);

reg [DATA_WIDTH-1:0] rf [2**REG_WIDTH-1:0];
always @(posedge clk) begin
	if (wen) 
		rf[waddr] <= wdata;
end

assign rdata1 = (raddr1 == 'b0) ? 'b0 : rf[raddr1];
assign rdata2 = (raddr2 == 'b0) ? 'b0 : rf[raddr2];

endmodule
