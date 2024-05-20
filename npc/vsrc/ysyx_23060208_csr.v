// CSRs
// mcause
module ysys_2306208_mcause
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
		input clk,
		input rst,
		input [DATA_WIDTH-1:0] wdata,
		
		output [DATA_WIDTH-1:0] rdata,
		input wen
);
reg [DATA_WIDTH-1:0] mcause_r;
always @(posedge clk) begin
	if (rst) 
		mcause_r <= 0;
	else if(wen)
		mcause_r <= wdata;
end
assign rdata = mcause_r;
endmodule

// mepc
module ysys_2306208_mepc
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

// mtvec
module ysys_2306208_mtvec
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
		input clk,
		input rst,
		input [DATA_WIDTH-1:0] wdata,
		
		output [DATA_WIDTH-1:0] rdata,
		input wen
);
reg [DATA_WIDTH-1:0] mtvec_r;
always @(posedge clk) begin
	if (rst) 
		mtvec_r <= 0;
	else if(wen)
		mtvec_r <= wdata;
end
assign rdata = mtvec_r;
endmodule

// mstatus
module ysys_2306208_mstatus
	#(REG_WIDTH = 5, DATA_WIDTH = 32) (
		input clk,
		input rst,
		input [DATA_WIDTH-1:0] wdata,
		
		output [DATA_WIDTH-1:0] rdata,
		input wen
);
reg [DATA_WIDTH-1:0] mstatus_r;
always @(posedge clk) begin
	if (rst) 
		mstatus_r <= 0;
	else if(wen)
		mstatus_r <= wdata;
end
assign rdata = mstatus_r;
endmodule
