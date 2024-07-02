// mstatus
module ysyx_23060208_mstatus
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
		input clock,
		input reset,
		input [DATA_WIDTH-1:0] wdata,
		
		output [DATA_WIDTH-1:0] rdata,
		input wen
);
reg [DATA_WIDTH-1:0] mstatus_r;
always @(posedge clock) begin
	if (reset) 
		mstatus_r <= 0;
	else if(wen)
		mstatus_r <= wdata;
end
assign rdata = mstatus_r;
endmodule
