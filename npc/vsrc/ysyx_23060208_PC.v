// pc register holds the address of the current instruction
// wen 始终有效
module ysyx_23060208_PC
	#(DATA_WIDTH = 32) (
<<<<<<< HEAD
	input clk,
	input rst,
	input 									wen,
	input  [DATA_WIDTH-1:0] next_pc,
=======
	input clock,
	input reset,
	input 									wen,
	input  [DATA_WIDTH-1:0] next_pc,

>>>>>>> tracer-ysyx
	output [DATA_WIDTH-1:0] pc
);

	
reg [DATA_WIDTH-1:0] pc_r;
<<<<<<< HEAD
always @(posedge clk) begin
	if (rst) 
		pc_r <= DATA_WIDTH'('h8000_0000) - DATA_WIDTH'('h4);
=======
always @(posedge clock) begin
	if (reset) 
		//pc_r <= DATA_WIDTH'('h8000_0000) - DATA_WIDTH'('h4);
		//pc_r <= DATA_WIDTH'('h8000_0000);
		// mrom addr
		//pc_r <= DATA_WIDTH'('h2000_0000) - DATA_WIDTH'('h4);
		pc_r <= DATA_WIDTH'('h3000_0000) - DATA_WIDTH'('h4);
>>>>>>> tracer-ysyx
	else if(wen)
		pc_r <= next_pc;
end
assign pc = pc_r;

endmodule
