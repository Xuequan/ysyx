// pc register holds the address of the current instruction
// wen 始终有效
module ysyx_23060208_PC
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,
	input 									wen,
	input  [DATA_WIDTH-1:0] next_pc,

	output [DATA_WIDTH-1:0] pc
);

	
reg [DATA_WIDTH-1:0] pc_r;
always @(posedge clk) begin
	if (rst) 
		pc_r <= DATA_WIDTH'('h8000_0000) - DATA_WIDTH'('h4);
		//pc_r <= DATA_WIDTH'('h8000_0000);
	else if(wen)
		pc_r <= next_pc;
end
assign pc = pc_r;

endmodule
