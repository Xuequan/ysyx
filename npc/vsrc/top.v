`include "ysyx_23060208_npc.h"
module top
	#(DATA_WIDTH = 32, REG_WIDTH = 5)(
	input clk,
	input rst
);

wire [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus;
wire											 exu_to_ifu_valid;
wire [`IFU_TO_IDU_BUS-1:0] ifu_to_idu_bus;

wire											 ifu_to_idu_valid;

wire											 idu_allowin;
wire											 ifu_allowin;
wire											 exu_allowin;


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

wire [DATA_WIDTH-1:0] dsram_awaddr_o;
wire                  dsram_awvalid_o;
wire                   dsram_awready_o;

wire [DATA_WIDTH-1:0] dsram_wdata_o; 
wire [2           :0] dsram_wstrb_o;
wire                  dsram_wvalid_o;
wire                   dsram_wready_o;

wire  [1            :0] dsram_bresp_o;
wire                   dsram_bvalid_o;
wire                   dsram_bready_o;

wire [DATA_WIDTH-1:0] dsram_araddr;
wire                  dsram_arvalid; 
wire                   dsram_arready;

wire [DATA_WIDTH-1:0]  dsram_rdata;  
wire [1:           0]  dsram_rresp;
wire                  dsram_rvalid;
wire                  dsram_rready;

wire [DATA_WIDTH-1:0] isram_araddr;
wire                  isram_arvalid; 
wire                   isram_arready;

wire [DATA_WIDTH-1:0]  isram_rdata;  
wire [1:           0]  isram_rresp;
wire                  isram_rvalid;
wire                  isram_rready;


wire [DATA_WIDTH-1:0] dsram_araddr_o;
wire                  dsram_arvalid_o; 
wire                   dsram_arready_o;

wire [DATA_WIDTH-1:0]  dsram_rdata_o;  
wire [1:           0]  dsram_rresp_o;
wire                  dsram_rvalid_o;
wire                  dsram_rready_o;

wire [DATA_WIDTH-1:0] isram_araddr_o;
wire                  isram_arvalid_o; 
wire                   isram_arready_o;

wire [DATA_WIDTH-1:0]  isram_rdata_o;  
wire [1:           0]  isram_rresp_o;
wire                  isram_rvalid_o;
wire                  isram_rready_o;
wire	idu_valid;

//wire [2						 :0] grant;
wire [1							 :0] exu_done;
wire									 ifu_done;

ysyx_23060208_arbiter	#(.DATA_WIDTH(DATA_WIDTH)) arbiter(
	.clk(clk),
	.rst(rst),

	.ifu_done(ifu_done),
	.exu_done(exu_done),
	//.grant(grant),

	// dsram
	.dsram_awaddr_i(dsram_awaddr),
	.dsram_awvalid_i(dsram_awvalid),
	.dsram_awready_i(dsram_awready),

	.dsram_wdata_i(dsram_wdata),
	.dsram_wstrb_i(dsram_wstrb),
	.dsram_wvalid_i(dsram_wvalid),
	.dsram_wready_i(dsram_wready),
	
	.dsram_bresp_i(dsram_bresp),
	.dsram_bvalid_i(dsram_bvalid),
	.dsram_bready_i(dsram_bready),

	.dsram_awaddr_o(dsram_awaddr_o),
	.dsram_awvalid_o(dsram_awvalid_o),
	.dsram_awready_o(dsram_awready_o),

	.dsram_wdata_o(dsram_wdata_o),
	.dsram_wstrb_o(dsram_wstrb_o),
	.dsram_wvalid_o(dsram_wvalid_o),
	.dsram_wready_o(dsram_wready_o),
	
	.dsram_bresp_o(dsram_bresp_o),
	.dsram_bvalid_o(dsram_bvalid_o),
	.dsram_bready_o(dsram_bready_o),

	.dsram_araddr_i(dsram_araddr),
	.dsram_arvalid_i(dsram_arvalid),
	.dsram_arready_i(dsram_arready),

	.dsram_rdata_i(dsram_rdata),
	.dsram_rresp_i(dsram_rresp),
	.dsram_rvalid_i(dsram_rvalid),
	.dsram_rready_i(dsram_rready),

	.dsram_araddr_o(dsram_araddr_o),
	.dsram_arvalid_o(dsram_arvalid_o),
	.dsram_arready_o(dsram_arready_o),

	.dsram_rdata_o(dsram_rdata_o),
	.dsram_rresp_o(dsram_rresp_o),
	.dsram_rvalid_o(dsram_rvalid_o),
	.dsram_rready_o(dsram_rready_o),

	// isram
	.isram_araddr_o(isram_araddr_o),
	.isram_arvalid_o(isram_arvalid_o),
	.isram_arready_o(isram_arready_o),

	.isram_rdata_o(isram_rdata_o),
	.isram_rresp_o(isram_rresp_o),
	.isram_rvalid_o(isram_rvalid_o),
	.isram_rready_o(isram_rready_o),
	
	.isram_araddr_i(isram_araddr),
	.isram_arvalid_i(isram_arvalid),
	.isram_arready_i(isram_arready),

	.isram_rdata_i(isram_rdata),
	.isram_rresp_i(isram_rresp),
	.isram_rvalid_i(isram_rvalid),
	.isram_rready_i(isram_rready)
);

ysyx_23060208_isram	#(.DATA_WIDTH(DATA_WIDTH)) isram(
	.clk(clk),
	.rst(rst),
	.ifu_allowin(ifu_allowin),

	.isram_araddr(isram_araddr_o),
	.isram_arvalid(isram_arvalid_o),
	.isram_arready(isram_arready),

	.isram_rdata(isram_rdata),
	.isram_rresp(isram_rresp),
	.isram_rvalid(isram_rvalid),
	.isram_rready(isram_rready_o)
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

	.exu_allowin(exu_allowin),

	.ifu_to_idu_bus(ifu_to_idu_bus),
	.ifu_to_idu_valid(ifu_to_idu_valid),
	.idu_valid(idu_valid),
	
	//.grant(grant),
	.ifu_done(ifu_done),

	.isram_araddr(isram_araddr),
	.isram_arvalid(isram_arvalid),
	.isram_arready(isram_arready_o),

	.isram_rdata(isram_rdata_o),
	.isram_rresp(isram_rresp_o),
	.isram_rvalid(isram_rvalid_o),
	.isram_rready(isram_rready),
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
	.idu_valid_o(idu_valid),
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
	
	//.grant(grant),
	.exu_done(exu_done),

	.dsram_awaddr(dsram_awaddr),
	.dsram_awvalid(dsram_awvalid),
	.dsram_awready(dsram_awready_o),

	.dsram_wdata(dsram_wdata),
	.dsram_wstrb(dsram_wstrb),
	.dsram_wvalid(dsram_wvalid),
	.dsram_wready(dsram_wready_o),
	
	.dsram_bresp(dsram_bresp_o),
	.dsram_bvalid(dsram_bvalid_o),
	.dsram_bready(dsram_bready),

	.dsram_araddr(dsram_araddr),
	.dsram_arvalid(dsram_arvalid),
	.dsram_arready(dsram_arready_o),

	.dsram_rdata(dsram_rdata_o),
	.dsram_rresp(dsram_rresp_o),
	.dsram_rvalid(dsram_rvalid_o),
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
	.dsram_awaddr(dsram_awaddr_o),
	.dsram_awvalid(dsram_awvalid_o),
	.dsram_awready(dsram_awready),

	.dsram_wdata(dsram_wdata_o),
	.dsram_wstrb(dsram_wstrb_o),
	.dsram_wvalid(dsram_wvalid_o),
	.dsram_wready(dsram_wready),
	
	.dsram_bresp(dsram_bresp),
	.dsram_bvalid(dsram_bvalid),
	.dsram_bready(dsram_bready_o),

	.dsram_araddr(dsram_araddr_o),
	.dsram_arvalid(dsram_arvalid_o),
	.dsram_arready(dsram_arready),

	.dsram_rdata(dsram_rdata),
	.dsram_rresp(dsram_rresp),
	.dsram_rvalid(dsram_rvalid),
	.dsram_rready(dsram_rready_o)
);

endmodule
