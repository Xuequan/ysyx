`include "ysyx_23060208_npc.h"
module top
	#(DATA_WIDTH = 32, REG_WIDTH = 5)(
	input clk,
	input rst,
	output [DATA_WIDTH-1:0] inst,
	output [DATA_WIDTH-1:0] pc
);

wire [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus;
wire											 exu_to_ifu_valid;
wire [`IFU_TO_IDU_BUS-1:0] ifu_to_idu_bus;
wire											 ifu_to_idu_valid;

wire											 idu_allowin;
wire											 ifu_allowin;
wire											 exu_allowin;

wire [DATA_WIDTH-1:0] isram_rdata;
wire [DATA_WIDTH-1:0] isram_raddr;
wire									isram_ready;

wire [DATA_WIDTH-1:0] regfile_wdata;
wire [REG_WIDTH-1 :0] regfile_waddr;
wire 									regfile_wen;

wire [11:0]           csr_raddr;
wire [DATA_WIDTH-1:0] csr_rdata;
wire [11:0]           csr_waddr;
wire [11:0]           csr_waddr2;
wire [DATA_WIDTH-1:0] csr_wdata;
wire [DATA_WIDTH-1:0] csr_wdata2;

wire [`IDU_TO_EXU_ALU_BUS-1:0] idu_to_exu_alu_bus;
wire [`IDU_TO_EXU_BUS-1		 :0] idu_to_exu_bus;
wire [`IDU_TO_EXU_CSR_BUS-1:0] idu_to_exu_csr_bus;

wire	idu_to_exu_valid;

wire [DATA_WIDTH-1:0] dsram_awaddr;
wire                  dsram_awvalid;
wire                   dsram_awready;

wire [DATA_WIDTH-1:0] dsram_wdata; 
wire [2           :0] dsram_wstrb;
wire                  dsram_wvalid;
wire                   dsram_wready;

wire  [1            :0] dsram_bresp;
wire                   dsram_bvalid;
wire                   dsram_bready;

wire [DATA_WIDTH-1:0] dsram_araddr;
wire                  dsram_arvalid; 
wire                   dsram_arready;

wire [DATA_WIDTH-1:0]  dsram_rdata;  
wire [1:           0]  dsram_rresp;
wire                  dsram_rvalid;
wire                  dsram_rready;

ysyx_23060208_isram	#(.DATA_WIDTH(DATA_WIDTH)) isram(
	.clk(clk),
	.rst(rst),
	.ifu_allowin(ifu_allowin),
	.isram_raddr(isram_raddr),
	.isram_ready(isram_ready),
	.isram_rdata(isram_rdata)
);
ysyx_23060208_CSR	#(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) csr(
	.clk(clk),
	.rst(rst),

	.csr_raddr(csr_raddr),
	.csr_rdata(csr_rdata),
	
	.csr_waddr(csr_waddr),
	.csr_wdata(csr_wdata),
	.csr_waddr2(csr_waddr2),
	.csr_wdata2(csr_wdata2)
);

ysyx_23060208_IFU #(.DATA_WIDTH(DATA_WIDTH)) ifu(
	.clk(clk),
	.rst(rst),

	.exu_to_ifu_bus(exu_to_ifu_bus),
	.exu_to_ifu_valid(exu_to_ifu_valid),

	.ifu_to_idu_bus(ifu_to_idu_bus),
	.ifu_to_idu_valid(ifu_to_idu_valid),
	.idu_allowin(idu_allowin),
	
	.isram_rdata(isram_rdata),
	.isram_ready(isram_ready),
	.isram_raddr(isram_raddr),

	.ifu_allowin(ifu_allowin)
);	

ysyx_23060208_IDU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) idu(
	.clk(clk),
	.rst(rst),

	.ifu_to_idu_bus(ifu_to_idu_bus),
	.ifu_to_idu_valid(ifu_to_idu_valid),

	.csr_raddr(csr_raddr),
	.csr_rdata(csr_rdata),

	.regfile_wdata(regfile_wdata),
	.regfile_waddr(regfile_waddr),
	.regfile_wen(regfile_wen),

	.idu_to_exu_alu_bus(idu_to_exu_alu_bus),
	.idu_to_exu_bus(idu_to_exu_bus),
	.idu_to_exu_csr_bus(idu_to_exu_csr_bus),

	.idu_to_exu_valid(idu_to_exu_valid),
	.exu_allowin(exu_allowin),
	.idu_allowin(idu_allowin)
);

ysyx_23060208_EXU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) exu(
	.clk(clk),
	.rst(rst),

	.exu_to_ifu_bus(exu_to_ifu_bus),
	.exu_to_ifu_valid(exu_to_ifu_valid),

	.regfile_wdata(regfile_wdata),
	.regfile_waddr(regfile_waddr),
	.regfile_wen(regfile_wen),
	
	.dsram_awaddr(dsram_awaddr),
	.dsram_awvalid(dsram_awvalid),
	.dsram_awready(dsram_awready),

	.dsram_wdata(dsram_wdata),
	.dsram_wstrb(dsram_wstrb),
	.dsram_wvalid(dsram_wvalid),
	.dsram_wready(dsram_wready),
	
	.dsram_bresp(dsram_bresp),
	.dsram_bvalid(dsram_bvalid),
	.dsram_bready(dsram_bready),

	.dsram_araddr(dsram_araddr),
	.dsram_arvalid(dsram_arvalid),
	.dsram_arready(dsram_arready),

	.dsram_rdata(dsram_rdata),
	.dsram_rresp(dsram_rresp),
	.dsram_rvalid(dsram_rvalid),
	.dsram_rready(dsram_rready),

	.csr_waddr(csr_waddr),
	.csr_wdata(csr_wdata),
	.csr_waddr2(csr_waddr2),
	.csr_wdata2(csr_wdata2),

	.idu_to_exu_alu_bus(idu_to_exu_alu_bus),
	.idu_to_exu_bus(idu_to_exu_bus),
	.idu_to_exu_csr_bus(idu_to_exu_csr_bus),

	.idu_to_exu_valid(idu_to_exu_valid),
	.exu_allowin(exu_allowin)
);

ysyx_23060208_dsram	#(.DATA_WIDTH(DATA_WIDTH)) dsram(
	.clk(clk),
	.rst(rst),
	.dsram_awaddr(dsram_awaddr),
	.dsram_awvalid(dsram_awvalid),
	.dsram_awready(dsram_awready),

	.dsram_wdata(dsram_wdata),
	.dsram_wstrb(dsram_wstrb),
	.dsram_wvalid(dsram_wvalid),
	.dsram_wready(dsram_wready),
	
	.dsram_bresp(dsram_bresp),
	.dsram_bvalid(dsram_bvalid),
	.dsram_bready(dsram_bready),

	.dsram_araddr(dsram_araddr),
	.dsram_arvalid(dsram_arvalid),
	.dsram_arready(dsram_arready),

	.dsram_rdata(dsram_rdata),
	.dsram_rresp(dsram_rresp),
	.dsram_rvalid(dsram_rvalid),
	.dsram_rready(dsram_rready)
);

endmodule
