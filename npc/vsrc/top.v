module top
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32, 
		REG_WIDTH = 5)(
	input clk,
	input rst,
	// to ram	
	output [ADDR_WIDTH-1:0] pc, 
	// from ram
	input  [DATA_WIDTH-1:0] inst
);

//wire [DATA_WIDTH-1:0] inst;
wire [DATA_WIDTH-1:0] wdata;
wire [REG_WIDTH-1 :0] waddr;
wire [DATA_WIDTH-1:0] src1;
wire [DATA_WIDTH-1:0] src2;
wire [REG_WIDTH-1 :0] rd;
wire [2						:0] op;


ysyx_23060208_IFU #(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH)) ifu(
	.clk(clk),
	.rst(rst),
	.inst_i(inst),
	.addr(pc)
);	

ysyx_23060208_IDU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) idu(
	.clk(clk),
	.rst(rst),
	.inst(inst),
	.wdata(wdata),
	.waddr(waddr),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op)
);

ysyx_23060208_EXU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) exu(
	.clk(clk),
	.rst(rst),
	.src1(src1),
	.src2(src2),
	.rd(rd),
	.op(op),
	.waddr(waddr),
	.wdata(wdata)
);
endmodule
