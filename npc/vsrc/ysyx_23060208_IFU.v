// IFU 模块
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,
	input [DATA_WIDTH-1:0]  inst_i,  // from inst ram

	// from EXU
	input [DATA_WIDTH-1:0]  exu_nextpc,
	input 									exu_nextpc_taken,

	output reg 							valid, // to isram						
	output [DATA_WIDTH-1:0] pc,  // to IDU
	output [DATA_WIDTH-1:0] nextPC    // to inst ram
);

assign nextPC = exu_nextpc_taken ? exu_nextpc :
													pc + 4;

ysyx_23060208_PC #(.DATA_WIDTH(DATA_WIDTH)) PC_i0(
	.clk(clk),
	.rst(rst),
	.wen(1'b1),
	.next_pc(nextPC),
	.pc(pc)
);

assign valid = 1'b1;

/* ==================== DPI-C ====================== */
export "DPI-C" task get_nextPC;
task get_nextPC (output [DATA_WIDTH-1:0] o);
	o = nextPC;
endtask
endmodule
