// CSRs
// mcause
module ysyx_23060208_mcause
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
		input clock,
		input reset,
		input [DATA_WIDTH-1:0] wdata,
		
		output [DATA_WIDTH-1:0] rdata,
		input wen
);
reg [DATA_WIDTH-1:0] mcause_r;
always @(posedge clock) begin
	if (reset) 
		mcause_r <= 0;
	else if(wen)
		mcause_r <= wdata;
end
assign rdata = mcause_r;
endmodule
