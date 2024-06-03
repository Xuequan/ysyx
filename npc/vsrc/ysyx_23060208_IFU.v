// IFU 模块
`include "ysyx_23060208_npc.h"    
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,

	/* connect with EXU */
	input [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus,
	input												exu_to_ifu_valid,

	/* connect with IDU */
	output [`IFU_TO_IDU_BUS-1:0] ifu_to_idu_bus,  
	output 											 ifu_to_idu_valid,
	input												 idu_allowin,

	/* connect with isram */
	input [DATA_WIDTH-1:0]  isram_rdata,  
	input										isram_ready,
	output [DATA_WIDTH-1:0] isram_raddr,

	output									ifu_allowin
);

// ifu_valid 表示当前 IFU 有有效的数据
wire ifu_valid;
// 表示当前取到的 instruction 有效
reg has_inst_valid;
// 表示 EXU 传过来的数据有效
reg exu_data_valid;

assign ifu_valid = has_inst_valid;

reg ifu_ready_go;
assign ifu_ready_go = 1'b1;

always @(posedge clk) begin
	if (rst)  
	 has_inst_valid <= 0;
	else if(ifu_allowin) 	 
	 has_inst_valid <= isram_ready;
end

always @(posedge clk) begin
	if (rst)  
	 exu_data_valid <= 0;
	else if(ifu_allowin) 	 
	 exu_data_valid <= exu_to_ifu_valid;
end

assign ifu_to_idu_valid = ifu_valid && ifu_ready_go;
//assign ifu_allowin = !ifu_valid || (ifu_ready_go && idu_allowin);
assign ifu_allowin = ~idu_allowin;

/* reveive instruction from isram */
reg [DATA_WIDTH-1:0] inst_r;
always @(posedge clk) begin
	if (rst)
		inst_r <= 0;
	else if (ifu_allowin && isram_ready) 
		inst_r <= isram_rdata;
end

/* handle data from exu */
wire [DATA_WIDTH-    1:0] exu_nextpc;
wire											exu_nextpc_taken;
reg [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus_r;
assign {exu_nextpc_taken, exu_nextpc} = exu_to_ifu_bus_r;

always @(posedge clk) begin
	if (rst) 
		exu_to_ifu_bus_r <= 0;
	else if (ifu_allowin)
		exu_to_ifu_bus_r <= exu_to_ifu_bus;
end

/* ====================  get the nextPC ================*/
wire [DATA_WIDTH-1:0] pc;
wire [DATA_WIDTH-1:0] nextPC;
assign isram_raddr = nextPC;
assign nextPC = (exu_nextpc_taken && exu_data_valid) ? exu_nextpc :
													pc + 4;

/* get PC from register PC */
ysyx_23060208_PC #(.DATA_WIDTH(DATA_WIDTH)) PC_i0(
	.clk(clk),
	.rst(rst),
	.wen(1'b1),
	.next_pc(nextPC),
	.pc(pc)
);

assign ifu_to_idu_bus = {pc, inst_r};

/* ==================== DPI-C ====================== */
export "DPI-C" task get_nextPC;
task get_nextPC (output [DATA_WIDTH-1:0] o);
	o = nextPC;
endtask
endmodule
