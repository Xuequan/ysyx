// register file
module ysyx_23060208_regfile 
	// REG_WIDTH: regfile address width
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
<<<<<<< HEAD
	input clk,
	input rst,
=======
	input clock,
	input reset,
>>>>>>> tracer-ysyx
	input [DATA_WIDTH-1:0] wdata,
	input [REG_WIDTH-1:0] waddr,

	output [DATA_WIDTH-1:0] rdata1,
	input  [REG_WIDTH-1:0] raddr1,

	output [DATA_WIDTH-1:0] rdata2,
	input  [REG_WIDTH-1:0] raddr2,
	input wen
);

reg [DATA_WIDTH-1:0] rf [2**REG_WIDTH-1:0];
<<<<<<< HEAD
always @(posedge clk) begin
	if (rst)
=======
always @(posedge clock) begin
	if (reset)
>>>>>>> tracer-ysyx
		rf[waddr] <= 0;
	else if (wen) 
		rf[waddr] <= (waddr == 'b0) ? 'b0 : wdata;
end

assign rdata1 = (raddr1 == 'b0) ? 'b0 : rf[raddr1];
assign rdata2 = (raddr2 == 'b0) ? 'b0 : rf[raddr2];

endmodule
