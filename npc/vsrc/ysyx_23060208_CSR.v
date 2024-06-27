// 处理CSR相关的模块
module ysyx_23060208_CSR
	#(DATA_WIDTH = 32, REG_WIDTH = 5) (
	input clk,
	input rst,

	/* read from CSR, connect with IDU */
	input  [11:0] 					csr_raddr,
	output [DATA_WIDTH-1:0] csr_rdata,

	/* write to CSR, connect with EXU */
	input [11:0]           csr_waddr,
	input [DATA_WIDTH-1:0] csr_wdata,
		// just write to mcause
	input [11:0]           csr_waddr2,
	input [DATA_WIDTH-1:0] csr_wdata2
);

wire mtvec_wen;
wire [DATA_WIDTH-1:0] mtvec_rdata;

ysyx_23060208_mtvec #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mtvec(
	.clk(clk),
	.rst(rst),
	.wdata(csr_wdata),
	.rdata(mtvec_rdata),
	.wen(mtvec_wen)
);

wire mcause_wen;
wire [DATA_WIDTH-1:0] mcause_rdata;
wire [DATA_WIDTH-1:0] mcause_wdata;

ysyx_23060208_mcause #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mcause(
	.clk(clk),
	.rst(rst),
	.wdata(mcause_wdata),
	.rdata(mcause_rdata),
	.wen(mcause_wen)
);

wire mepc_wen;
wire [DATA_WIDTH-1:0] mepc_rdata;

ysyx_23060208_mepc #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mepc(
	.clk(clk),
	.rst(rst),
	.wdata(csr_wdata),
	.rdata(mepc_rdata),
	.wen(mepc_wen)
);

wire mstatus_wen;
wire [DATA_WIDTH-1:0] mstatus_rdata;
ysyx_23060208_mstatus #(.REG_WIDTH(REG_WIDTH), .DATA_WIDTH(DATA_WIDTH)) mstatus(
	.clk(clk),
	.rst(rst),
	.wdata(csr_wdata),
	.rdata(mstatus_rdata),
	.wen(mstatus_wen)
);

/* ================ write to CSRs ============= */
assign mtvec_wen   = csr_waddr == 12'h305;
assign mepc_wen    = csr_waddr == 12'h341;
assign mstatus_wen = csr_waddr == 12'h300;
assign mcause_wen  = (csr_waddr == 12'h342) || (csr_waddr2 == 12'h342);
assign mcause_wdata = (csr_waddr2 == 12'h342) ? csr_wdata2 : csr_wdata; 
														

/* ================ read from CSRs ============= */
assign csr_rdata =  ({DATA_WIDTH{csr_raddr == 12'h305}} & mtvec_rdata) 
									| ({DATA_WIDTH{csr_raddr == 12'h341}} & mepc_rdata)
									| ({DATA_WIDTH{csr_raddr == 12'h342}} & mcause_rdata)
									| ({DATA_WIDTH{csr_raddr == 12'h300}} & mstatus_rdata);

endmodule
