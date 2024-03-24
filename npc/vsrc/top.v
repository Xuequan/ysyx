module top
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32, 
		REG_WIDTH = 5)(
	input clk,
	input rst,
	output [DATA_WIDTH-1:0] inst,
	output inst_ebreak
);

wire [DATA_WIDTH-1:0] wdata;
wire [REG_WIDTH-1 :0] waddr;
wire [DATA_WIDTH-1:0] src1;
wire [DATA_WIDTH-1:0] src2;
wire [REG_WIDTH-1 :0] rd;
wire [2						:0] op;

wire [DATA_WIDTH-1:0] addr;
wire [ADDR_WIDTH-1:0] pc;
wire [ADDR_WIDTH-1:0] pc_o;
wire 									valid;

wire 									inst_jal_jalr;
wire [DATA_WIDTH-1:0] nextpc_from_jal_jalr;

ysyx_23060208_isram	#(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH)) isram_i0(
	.clk(clk),
	.rst(rst),
	.valid(valid),
	.raddr(addr),
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
	.inst_jal_jalr(inst_jal_jalr),
	.pc_o(pc_o),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op),
	.inst_ebreak(inst_ebreak)
);

ysyx_23060208_EXU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) exu(
	.clk(clk),
	.rst(rst),
	.inst_jal_jalr(inst_jal_jalr),
	.pc(pc_o),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op),
	.nextpc_from_jal_jalr(nextpc_from_jal_jalr),
	.waddr(waddr),
	.wdata(wdata)
);

export "DPI-C" task check_ebreak;
task check_ebreak (output bit o);
	o = inst_ebreak;
endtask

endmodule
