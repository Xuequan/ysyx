`include "ysyx_23060208_npc.h"
module ysyx_23060208
	#(DATA_WIDTH = 32, REG_WIDTH = 5)(
	input clock,
	input reset,

  input io_interrupt,  // src/CPU.scala:35:23, :36:19, :38:21                           
  input                       io_master_awready,                
  output                      io_master_awvalid,
  output [DATA_WIDTH-1    :0] io_master_awaddr,
  output [3               :0] io_master_awid,
  output [7               :0] io_master_awlen,
  output [2               :0] io_master_awsize,
  output [1               :0] io_master_awburst,
  
  input                       io_master_wready,
  output                      io_master_wvalid,
  output [DATA_WIDTH * 2-1:0] io_master_wdata,
  output [7               :0] io_master_wstrb,
  output                      io_master_wlast,

  input                       io_master_bvalid,
  output                      io_master_bready,
  input  [1               :0] io_master_bresp,
  input  [3               :0] io_master_bid,


  input                       io_master_arready,
  output                      io_master_arvalid,
  output [DATA_WIDTH-1    :0] io_master_araddr,
  output [3               :0] io_master_arid,
  output [7               :0] io_master_arlen,
  output [2               :0] io_master_arsize,
  output [1               :0] io_master_arburst,

  output                      io_master_rready,
  input                       io_master_rvalid,
  input  [1               :0] io_master_rresp,
  input  [DATA_WIDTH * 2-1:0] io_master_rdata,
  input                       io_master_rlast,
  input  [3               :0] io_master_rid,


  output                     io_slave_awready,                
  input                      io_slave_awvalid,
  input [DATA_WIDTH-1    :0] io_slave_awaddr,
  input [3               :0] io_slave_awid,
  input [7               :0] io_slave_awlen,
  input [2               :0] io_slave_awsize,
  input [1               :0] io_slave_awburst,
  
  output                     io_slave_wready,
  input                      io_slave_wvalid,
  input [DATA_WIDTH * 2-1:0] io_slave_wdata,
  input [7               :0] io_slave_wstrb,
  input                      io_slave_wlast,

  output                     io_slave_bvalid,
  input                      io_slave_bready,
  output  [1               :0] io_slave_bresp,
  output  [3               :0] io_slave_bid,


  output                     io_slave_arready,
  input                      io_slave_arvalid,
  input [DATA_WIDTH-1    :0] io_slave_araddr,
  input [3               :0] io_slave_arid,
  input [7               :0] io_slave_arlen,
  input [2               :0] io_slave_arsize,
  input [1               :0] io_slave_arburst,

  input                      io_slave_rready,
  output                     io_slave_rvalid,
  output [1               :0] io_slave_rresp,
  output [DATA_WIDTH * 2-1:0] io_slave_rdata,
  output                       io_slave_rlast,
  output [3               :0] io_slave_rid

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

wire [DATA_WIDTH-1:0] clint_araddr;
wire                  clint_arvalid; 
wire [3						:0] clint_arid;
wire [7						:0] clint_arlen;
wire [2						:0] clint_arsize;
wire [1						:0] clint_arburst;
wire                  clint_arready;

wire                  	clint_rvalid;
wire [DATA_WIDTH*2-1:0] clint_rdata;  
wire [1:           	 0] clint_rresp;
wire 										clint_rlast;
wire [3							:0] clint_rid;
wire                  	clint_rready;

wire                  dsram_awready;
wire                  dsram_awvalid;
wire [DATA_WIDTH-1:0] dsram_awaddr;
wire [3						:0] dsram_awid;
wire [7						:0] dsram_awlen;
wire [2						:0] dsram_awsize;
wire [1						:0] dsram_awburst;

wire                    dsram_wready;
wire                    dsram_wvalid;
wire [DATA_WIDTH*2-1:0] dsram_wdata; 
wire [7           	:0] dsram_wstrb;
wire										dsram_wlast;

wire                   dsram_bready;
wire                   dsram_bvalid;
wire  [1            :0] dsram_bresp;
wire	[3						:0] dsram_bid;


wire                  dsram_arready;
wire                  dsram_arvalid; 
wire [DATA_WIDTH-1:0] dsram_araddr;
wire [3						:0] dsram_arid;
wire [7						:0] dsram_arlen;
wire [2						:0] dsram_arsize;
wire [1						:0] dsram_arburst;

wire                    dsram_rready;
wire                    dsram_rvalid;
wire [1:             0] dsram_rresp;
wire [DATA_WIDTH*2-1:0] dsram_rdata;  
wire										dsram_rlast;
wire[3							:0]	dsram_rid;

wire                  isram_arready;
wire                  isram_arvalid; 
wire [DATA_WIDTH-1:0] isram_araddr;
wire [3						:0] isram_arid;
wire [7						:0] isram_arlen;
wire [2						:0] isram_arsize;
wire [1						:0] isram_arburst;

wire                    isram_rready;
wire                    isram_rvalid;
wire [1:             0] isram_rresp;
wire [DATA_WIDTH*2-1:0] isram_rdata;  
wire										isram_rlast;
wire [3							:0] isram_rid;

wire	     idu_valid;
wire [1:0] exu_done;
wire			 ifu_done;

ysyx_23060208_intercom	#(.DATA_WIDTH(DATA_WIDTH)) intercom(
	.clock(clock),
	.reset(reset),

	.ifu_done(ifu_done),
	.exu_done(exu_done),

	// EXU write
	.to_dsram_awready(dsram_awready),
	.from_dsram_awvalid(dsram_awvalid),
	.from_dsram_awaddr(dsram_awaddr),
	.from_dsram_awid(dsram_awid),
	.from_dsram_awlen(dsram_awlen),
	.from_dsram_awsize(dsram_awsize),
	.from_dsram_awburst(dsram_awburst),

	.to_dsram_wready(dsram_wready),
	.from_dsram_wvalid(dsram_wvalid),
	.from_dsram_wdata(dsram_wdata),
	.from_dsram_wstrb(dsram_wstrb),
	.from_dsram_wlast(dsram_wlast),
	
	.from_dsram_bready(dsram_bready),
	.to_dsram_bvalid(dsram_bvalid),
	.to_dsram_bresp(dsram_bresp),
	.to_dsram_bid(dsram_bid),

	// EXU read
	.to_dsram_arready(dsram_arready),
	.from_dsram_arvalid(dsram_arvalid),
	.from_dsram_araddr(dsram_araddr),
	.from_dsram_arid(dsram_arid),
	.from_dsram_arlen(dsram_arlen),
	.from_dsram_arsize(dsram_arsize),
	.from_dsram_arburst(dsram_arburst),

	.from_dsram_rready(dsram_rready),
	.to_dsram_rvalid(dsram_rvalid),
	.to_dsram_rresp(dsram_rresp),
	.to_dsram_rdata(dsram_rdata),
	.to_dsram_rlast(dsram_rlast),
	.to_dsram_rid(dsram_rid),

	// isram
	.to_isram_arready(isram_arready),
	.from_isram_arvalid(isram_arvalid),
	.from_isram_araddr(isram_araddr),
	.from_isram_arid(isram_arid),
	.from_isram_arlen(isram_arlen),
	.from_isram_arsize(isram_arsize),
	.from_isram_arburst(isram_arburst),

	.from_isram_rready(isram_rready),
	.to_isram_rvalid(isram_rvalid),
	.to_isram_rresp(isram_rresp),
	.to_isram_rdata(isram_rdata),
	.to_isram_rlast(isram_rlast),
	.to_isram_rid(isram_rid),

	// clint
	.from_clint_arready(clint_arready),
	.to_clint_arvalid(clint_arvalid),
	.to_clint_araddr(clint_araddr),
	.to_clint_arid(clint_arid),
	.to_clint_arlen(clint_arlen),
	.to_clint_arsize(clint_arsize),
	.to_clint_arburst(clint_arburst),

	.to_clint_rready(clint_rready),
	.from_clint_rvalid(clint_rvalid),
	.from_clint_rresp(clint_rresp),
	.from_clint_rdata(clint_rdata),
	.from_clint_rlast(clint_rlast),
	.from_clint_rid(clint_rid),

  .io_master_awready      (io_master_awready),
  .io_master_awvalid      (io_master_awvalid),
  .io_master_awid    (io_master_awid),
  .io_master_awaddr  (io_master_awaddr),
  .io_master_awlen   (io_master_awlen),
  .io_master_awsize  (io_master_awsize),
  .io_master_awburst (io_master_awburst),

  .io_master_wready       (io_master_wready),
  .io_master_wvalid       (io_master_wvalid),
  .io_master_wdata   (io_master_wdata),
  .io_master_wstrb   (io_master_wstrb),
  .io_master_wlast   (io_master_wlast),

  .io_master_bready       (io_master_bready),
  .io_master_bvalid       (io_master_bvalid),
  .io_master_bid     (io_master_bid),
  .io_master_bresp   (io_master_bresp),

  .io_master_arready      (io_master_arready),
  .io_master_arvalid      (io_master_arvalid),
  .io_master_arid    (io_master_arid),
  .io_master_araddr  (io_master_araddr),
  .io_master_arlen   (io_master_arlen),
  .io_master_arsize  (io_master_arsize),
  .io_master_arburst (io_master_arburst),

  .io_master_rready       (io_master_rready),
  .io_master_rvalid       (io_master_rvalid),
  .io_master_rid     (io_master_rid),
  .io_master_rdata   (io_master_rdata),
  .io_master_rresp   (io_master_rresp),
  .io_master_rlast   (io_master_rlast),

  .io_slave_awready       (/* unused */),
  .io_slave_awvalid       (1'h0),
  .io_slave_awid     (4'h0),
  .io_slave_awaddr   (32'h0),
  .io_slave_awlen    (8'h0),
  .io_slave_awsize   (3'h0),
  .io_slave_awburst  (2'h0),

  .io_slave_wready        (/* unused */),
  .io_slave_wvalid        (1'h0),
  .io_slave_wdata    (64'h0),
  .io_slave_wstrb    (8'h0),
  .io_slave_wlast    (1'h0),

  .io_slave_bready        (1'h0),
  .io_slave_bvalid        (/* unused */),
  .io_slave_bid      (/* unused */),
  .io_slave_bresp    (/* unused */),

  .io_slave_arready       (/* unused */),
  .io_slave_arvalid       (1'h0),
  .io_slave_arid     (4'h0),
  .io_slave_araddr   (32'h0),
  .io_slave_arlen    (8'h0),
  .io_slave_arsize   (3'h0),
  .io_slave_arburst  (2'h0),

  .io_slave_rready        (1'h0),
  .io_slave_rvalid        (/* unused */),
	.io_slave_rresp        (),
	.io_slave_rdata (),
	.io_slave_rlast(),
	.io_slave_rid()
);

ysyx_23060208_clint	#(.DATA_WIDTH(DATA_WIDTH)) clint(
	.clock(clock),
	.reset(reset),

	.clint_araddr(clint_araddr),
	.clint_arvalid(clint_arvalid),
	.clint_arid(clint_arid),
	.clint_arlen(clint_arlen),
	.clint_arsize(clint_arsize),
	.clint_arburst(clint_arburst),
	.clint_arready(clint_arready),

	.clint_rvalid(clint_rvalid),
	.clint_rdata(clint_rdata),
	.clint_rresp(clint_rresp),
	.clint_rlast(clint_rlast),
	.clint_rid(clint_rid),
	.clint_rready(clint_rready)
);

ysyx_23060208_CSR	#(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) csr(
	.clock(clock),
	.reset(reset),

	.csr_raddr(csr_raddr),
	.csr_rdata(csr_rdata),
	
	.csr_waddr(csr_waddr),
	.csr_wdata(csr_wdata),
	.csr_waddr2(csr_waddr2),
	.csr_wdata2(csr_wdata2)
);

ysyx_23060208_IFU #(.DATA_WIDTH(DATA_WIDTH)) ifu(
	.clock(clock),
	.reset(reset),

	.exu_to_ifu_bus(exu_to_ifu_bus),
	.exu_to_ifu_valid(exu_to_ifu_valid),

	.exu_allowin(exu_allowin),

	.ifu_to_idu_bus(ifu_to_idu_bus),
	.ifu_to_idu_valid(ifu_to_idu_valid),
	.idu_valid(idu_valid),
	
	.ifu_done(ifu_done),

	.isram_araddr(isram_araddr),
	.isram_arvalid(isram_arvalid),
	.isram_arready(isram_arready),
	.isram_arlen(isram_arlen),
	.isram_arid(isram_arid),
	.isram_arsize(isram_arsize),
	.isram_arburst(isram_arburst),

	.isram_rvalid(isram_rvalid),
	.isram_rready(isram_rready),
	.isram_rresp(isram_rresp),
	.isram_rdata(isram_rdata),
	.isram_rlast(isram_rlast),
	.isram_rid(isram_rid),

	.ifu_allowin(ifu_allowin)
);	

ysyx_23060208_IDU #(.DATA_WIDTH(DATA_WIDTH), .REG_WIDTH(REG_WIDTH)) idu(
	.clock(clock),
	.reset(reset),

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
	.clock(clock),
	.reset(reset),

	.exu_to_ifu_bus(exu_to_ifu_bus),
	.exu_to_ifu_valid(exu_to_ifu_valid),

	.regfile_wdata(regfile_wdata),
	.regfile_waddr(regfile_waddr),
	.regfile_wen(regfile_wen),
	
	.exu_done(exu_done),

	.axi_awready(dsram_awready),
	.axi_awaddr(dsram_awaddr),
	.axi_awvalid(dsram_awvalid),

	.axi_awid(dsram_awid),
	.axi_awlen(dsram_awlen),
	.axi_awsize(dsram_awsize),
	.axi_awburst(dsram_awburst),

	.axi_wvalid(dsram_wvalid),
	.axi_wdata(dsram_wdata),
	.axi_wstrb(dsram_wstrb),
	.axi_wlast(dsram_wlast),
	.axi_wready(dsram_wready),
	
	.axi_bready(dsram_bready),
	.axi_bvalid(dsram_bvalid),
	.axi_bresp(dsram_bresp),
	.axi_bid(dsram_bid),

	.axi_arready(dsram_arready),
	.axi_arvalid(dsram_arvalid),
	.axi_araddr(dsram_araddr),
	.axi_arid(dsram_arid),
	.axi_arlen(dsram_arlen),
	.axi_arsize(dsram_arsize),
	.axi_arburst(dsram_arburst),

	.axi_rready(dsram_rready),
	.axi_rvalid(dsram_rvalid),
	.axi_rresp(dsram_rresp),
	.axi_rdata(dsram_rdata),
	.axi_rlast(dsram_rlast),
	.axi_rid(dsram_rid),


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

endmodule
