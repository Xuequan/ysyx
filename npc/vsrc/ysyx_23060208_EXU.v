// EXU 模块
module ysyx_23060208_EXU
	#(DATA_WIDTH = 32, REG_WIDTH = 5, ADDR_WIDTH = 32) (
	input clk,
	input rst,

	input 								 inst_jal_jalr,
	input [DATA_WIDTH-1:0] pc,
	input [DATA_WIDTH-1:0] src1,
	input [DATA_WIDTH-1:0] src2,
	input [REG_WIDTH-1:0]  rd,
	input [2					 :0] op,

	input									 dest,
	input [DATA_WIDTH-1:0] to_mem_data_i,
	// to IFU for gen nextPC
	output [DATA_WIDTH-1:0] nextpc_from_jal_jalr,
	// output to register file
	output [REG_WIDTH-1:0] waddr,
	output								 wen,
	output [DATA_WIDTH-1:0] wdata,
	// output to memory
	output [ADDR_WIDTH-1:0] to_mem_addr,
	output									to_mem_wen,
	output [DATA_WIDTH-1:0] to_mem_data_o
);

wire [DATA_WIDTH-1:0] alu_result;
wire overflow;

ysyx_23060208_alu #(.DATA_WIDTH(DATA_WIDTH)) i0(
	.result(alu_result),
	.overflow(overflow),
	.op(op),
	.src1(src1),
	.src2(src2)
);

// 若是 jal, jalr, 那么将 rd <- pc + 4
assign wdata = inst_jal_jalr ? pc + 4 : alu_result;
assign waddr = rd;

assign nextpc_from_jal_jalr = alu_result;

assign wen = dest;
// write to memory
assign to_mem_addr = alu_result; 
assign to_mem_data_o = to_mem_data_i;
assign to_mem_wen = ~dest;

//=============== DPI-C =========================
export "DPI-C" task update_regfile_no;
task update_regfile_no (output [REG_WIDTH-1:0] reg_no);
	reg_no = wen ? waddr : 'b0;
endtask

export "DPI-C" task update_regfile_data;
task update_regfile_data (output [DATA_WIDTH-1:0] din);
	din    = wdata;
endtask

endmodule

