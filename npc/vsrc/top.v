module top
	#(DATA_WIDTH = 32, REG_WIDTH = 5)(
	input clk,
	input rst,
	output [DATA_WIDTH-1:0] inst,
	output [DATA_WIDTH-1:0] pc
);

wire [DATA_WIDTH-1:0] nextPC;
wire [DATA_WIDTH-1:0] regfile_wdata;
wire [REG_WIDTH-1 :0] regfile_waddr;
wire 									regfile_wen;

wire [1						:0] uncond_jump_inst;
wire 									exu_nextpc_taken_to_ifu;
wire [DATA_WIDTH-1:0] exu_nextpc_to_ifu;

wire [DATA_WIDTH-1:0] cond_branch_target;
wire 									cond_branch_inst;

wire [4						:0] load_inst;

wire [DATA_WIDTH-1:0] src1;
wire [DATA_WIDTH-1:0] src2;
wire [REG_WIDTH-1 :0] rd;
wire [17					:0] op;
wire [1           :0]	regfile_mem_mux;

wire [DATA_WIDTH-1:0] store_data;
wire [DATA_WIDTH-1:0] store_address;
wire									store_en;
wire [2						:0] store_inst;
wire [2						:0] store_bytes_num;
wire [DATA_WIDTH-1:0] store_data_raw;

wire [DATA_WIDTH-1:0] rdata;
wire [DATA_WIDTH-1:0] raddr;

wire [DATA_WIDTH-1:0] idu_to_exu_pc;
wire 									ifu_to_isram_valid;
wire									exu_to_dsram_read_en;

wire [DATA_WIDTH-1:0] csr_nextpc;
wire 									csr_nextpc_taken;
wire [DATA_WIDTH-1:0] csr_wdata;
wire [1						:0] csr_inst;

ysyx_23060208_isram	#(.DATA_WIDTH(DATA_WIDTH)) isram(
	.clk(clk),
	.rst(rst),
	.valid(ifu_to_isram_valid),
	.nextPC(nextPC),
	.inst_o(inst)
);

ysyx_23060208_IFU #(.DATA_WIDTH(DATA_WIDTH)) ifu(
	.clk(clk),
	.rst(rst),
	.inst_i(inst),

	.exu_nextpc(exu_nextpc_to_ifu),
	.exu_nextpc_taken(exu_nextpc_taken_to_ifu),

	.valid(ifu_to_isram_valid),
	.pc(pc),
	.nextPC(nextPC)
);	

ysyx_23060208_IDU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) idu(
	.clk(clk),
	.rst(rst),
	.inst(inst),
	.pc_i(pc),

	.regfile_wdata(regfile_wdata),
	.regfile_waddr(regfile_waddr),
	.regfile_wen(regfile_wen),

	.uncond_jump_inst(uncond_jump_inst),

	.pc_o(idu_to_exu_pc),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op),

	.regfile_mem_mux(regfile_mem_mux),
	.cond_branch_inst(cond_branch_inst),
	.cond_branch_target(cond_branch_target),

	.load_inst(load_inst),

	.store_inst(store_inst),
	.store_data_raw(store_data_raw),

	.csr_nextpc(csr_nextpc),
	.csr_nextpc_taken(csr_nextpc_taken),
	.csr_wdata(csr_wdata),
	.csr_inst(csr_inst)
);

ysyx_23060208_EXU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) exu(
	.clk(clk),
	.rst(rst),
	.uncond_jump_inst(uncond_jump_inst),
	.pc(idu_to_exu_pc),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op),

	.regfile_mem_mux(regfile_mem_mux),

	.cond_branch_target(cond_branch_target),
	.cond_branch_inst(cond_branch_inst),

	.load_inst(load_inst),

	.exu_nextpc(exu_nextpc_to_ifu),
	.exu_nextpc_taken(exu_nextpc_taken_to_ifu),

	.regfile_wdata(regfile_wdata),
	.regfile_waddr(regfile_waddr),
	.regfile_wen(regfile_wen),
	
	.store_inst(store_inst),
	.store_address(store_address),
	.store_en(store_en),
	.store_data_raw(store_data_raw),
	.store_data(store_data),
	.store_bytes_num(store_bytes_num),
	
	.rdata(rdata),
	.raddr(raddr),
	.valid(exu_to_dsram_read_en),

	.csr_nextpc(csr_nextpc),
	.csr_nextpc_taken(csr_nextpc_taken),
	.csr_inst(csr_inst),
	.csr_wdata(csr_wdata)
);

ysyx_23060208_dsram	#(.DATA_WIDTH(DATA_WIDTH)) dsram(
	.clk(clk),
	//.rst(rst),
	.valid(exu_to_dsram_read_en),
	.wdata(store_data),
	.waddr(store_address),
	.wen(store_en),
	.store_bytes_num(store_bytes_num),
	.raddr(raddr),
	.rdata(rdata)
);

endmodule
