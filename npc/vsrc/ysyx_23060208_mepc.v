// mepc
module ysyx_23060208_mepc
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
		input clk,
		input rst,
		input [DATA_WIDTH-1:0] wdata,
		
		output [DATA_WIDTH-1:0] rdata,
		input wen
);
reg [DATA_WIDTH-1:0] mepc_r;
always @(posedge clk) begin
	if (rst) 
		mepc_r <= 0;
	else if(wen)
		mepc_r <= wdata;
end
assign rdata = mepc_r;
endmodule
