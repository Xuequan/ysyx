module top
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32, 
		REG_WIDTH = 5)(
	input clk,
	input rst,
	output [DATA_WIDTH-1:0] inst,
	output [ADDR_WIDTH-1:0] pc,
	output inst_ebreak
);

wire [DATA_WIDTH-1:0] wdata;
wire [REG_WIDTH-1 :0] waddr;
wire [DATA_WIDTH-1:0] src1;
wire [DATA_WIDTH-1:0] src2;
wire [REG_WIDTH-1 :0] rd;
wire [2						:0] op;

wire [DATA_WIDTH-1:0] addr;
//wire [ADDR_WIDTH-1:0] pc;
wire [ADDR_WIDTH-1:0] idu_to_exu_pc;
wire 									valid;

wire 									inst_jal_jalr;
wire [DATA_WIDTH-1:0] nextpc_from_jal_jalr;

wire [DATA_WIDTH-1:0] exu_to_isram_to_mem_addr;
wire [DATA_WIDTH-1:0] exu_to_isram_to_mem_data;
wire									to_mem_wen;
wire									dest;
wire [DATA_WIDTH-1:0] idu_to_exu_to_mem_data;

wire 									to_regfile_wen;

ysyx_23060208_isram	#(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH)) isram_i0(
	.clk(clk),
	.rst(rst),
	.valid(valid),
	.raddr(addr),
	.waddr(exu_to_isram_to_mem_addr),
	.wdata(exu_to_isram_to_mem_data),
	.wen(to_mem_wen),
	.inst_o(inst)
);

ysyx_23060208_IFU #(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH)) ifu(
	.clk(clk),
	.rst(rst),
	.inst_i(inst),
	.inst_jal_jalr(inst_jal_jalr),
	.nextpc_from_jal_jalr(nextpc_from_jal_jalr),
	.valid(valid),
	.pc(pc),
	.addr(addr)
);	

ysyx_23060208_IDU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) idu(
	.clk(clk),
	.rst(rst),
	.inst(inst),
	.pc_i(pc),
	.wdata(wdata),
	.waddr(waddr),
	.wen(to_regfile_wen),

	.inst_jal_jalr(inst_jal_jalr),
	.pc_o(idu_to_exu_pc),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op),

	.dest(dest),
	.to_mem_data_o(idu_to_exu_to_mem_data),
	.inst_ebreak(inst_ebreak)
);

ysyx_23060208_EXU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH), .ADDR_WIDTH(ADDR_WIDTH)) exu(
	.clk(clk),
	.rst(rst),
	.inst_jal_jalr(inst_jal_jalr),
	.pc(idu_to_exu_pc),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op),

	.dest(dest),
	.to_mem_data_i(idu_to_exu_to_mem_data),

	.nextpc_from_jal_jalr(nextpc_from_jal_jalr),
	.waddr(waddr),
	.wdata(wdata),
	.wen(to_regfile_wen),
	
	.to_mem_addr(exu_to_isram_to_mem_addr),
	.to_mem_wen(to_mem_wen),
	.to_mem_data_o(exu_to_isram_to_mem_data)
);

/*
export "DPI-C" task check_ebreak;
task check_ebreak (output bit o);
	o = inst_ebreak;
endtask
*/

endmodule
