module ysyx_23060208_mtvec
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
		input clock,
		input reset,
		input [DATA_WIDTH-1:0] wdata,
		
		output [DATA_WIDTH-1:0] rdata,
		input wen
);
reg [DATA_WIDTH-1:0] mtvec_r;
always @(posedge clock) begin
	if (reset) 
		mtvec_r <= 0;
	else if(wen)
		mtvec_r <= wdata;
end
assign rdata = mtvec_r;
endmodule
