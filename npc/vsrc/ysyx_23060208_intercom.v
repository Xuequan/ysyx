module ysyx_23060208_intercom
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clock,
	input reset,
	
	input 								 ifu_done,
	input [1					 :0] exu_done,

	// ===================================================
	// EXU write
	// 写地址通道:
	output reg								  to_dsram_awready,
	input												from_dsram_awvalid,
	input [ADDR_WIDTH-1			:0] from_dsram_awaddr,
	input [3  							:0]	from_dsram_awid,
	input [7								:0] from_dsram_awlen,
	input [2								:0] from_dsram_awsize,
	input [1								:0] from_dsram_awburst,
	
	output reg									to_dsram_wready,
	input												from_dsram_wvalid,
	input [DATA_WIDTH * 2-1 :0] from_dsram_wdata,
	input [7					  		:0] from_dsram_wstrb,
	input												from_dsram_wlast,

	input														from_dsram_bready,
	output reg     								  to_dsram_bvalid,
	output reg [1					  		:0] to_dsram_bresp,
	output reg [3					  		:0] to_dsram_bid,

	// EXU read
	output reg											to_dsram_arready,
	input														from_dsram_arvalid,
	input [ADDR_WIDTH-1					:0] from_dsram_araddr,
	input [3  									:0]	from_dsram_arid,
	input [7										:0] from_dsram_arlen,
	input [2										:0] from_dsram_arsize,
	input [1									  :0] from_dsram_arburst,

	input														from_dsram_rready,
	output reg			 								to_dsram_rvalid,
	output reg [1					  		:0]	to_dsram_rresp,
	output reg [DATA_WIDTH * 2-1:0] to_dsram_rdata,
	output reg			 								to_dsram_rlast,
	output reg [3					  		:0]	to_dsram_rid,
	// ===================================================
	//
	// ===================================================
	// IFU read inst
	output reg											to_isram_arready,
	input														from_isram_arvalid,
	input [ADDR_WIDTH-1					:0] from_isram_araddr,
	input [3  									:0]	from_isram_arid,
	input [7										:0] from_isram_arlen,
	input [2										:0] from_isram_arsize,
	input [1									  :0] from_isram_arburst,

	input														from_isram_rready,
	output reg			 								to_isram_rvalid,
	output reg [1					  		:0]	to_isram_rresp,
	output reg [DATA_WIDTH * 2-1:0] to_isram_rdata,
	output reg			 								to_isram_rlast,
	output reg [3					  		:0]	to_isram_rid,
	// ===================================================
	// ===================================================
	// CLINT 
	input														from_clint_arready,

	output reg										  to_clint_arvalid,
	output reg [ADDR_WIDTH-1		:0] to_clint_araddr,
	output reg [3  							:0]	to_clint_arid,
	output reg [7								:0] to_clint_arlen,
	output reg [2								:0] to_clint_arsize,
	output reg [1								:0] to_clint_arburst,

	output reg											to_clint_rready,
	input			 											from_clint_rvalid,
	input [1					  		:0]			from_clint_rresp,
	input [DATA_WIDTH * 2-1 :0] 		from_clint_rdata,
	input			 											from_clint_rlast,
	input [3					  		:0]			from_clint_rid,

//=========================================
 // 对 npc CPU 之外
	input											  		io_master_awready,
	output reg											io_master_awvalid,
	output reg [ADDR_WIDTH-1		:0] io_master_awaddr,
	output reg [3  							:0]	io_master_awid,
	output reg [7								:0] io_master_awlen,
	output reg [2								:0] io_master_awsize,
	output reg [1								:0] io_master_awburst,
	
	input											  		io_master_wready,
	output reg											io_master_wvalid,
	output reg [DATA_WIDTH * 2-1:0] io_master_wdata,
	output reg [7					  		:0] io_master_wstrb,
	output reg											io_master_wlast,

	input     								  		io_master_bvalid,
	output reg											io_master_bready,
	input      [1					  		:0] io_master_bresp,
	input      [3					  		:0] io_master_bid,


	input											  		io_master_arready,
	output reg											io_master_arvalid,
	output reg [ADDR_WIDTH-1		:0] io_master_araddr,
	output reg [3  							:0]	io_master_arid,
	output reg [7								:0] io_master_arlen,
	output reg [2								:0] io_master_arsize,
	output reg [1								:0] io_master_arburst,

	output reg											io_master_rready,
	input			 											io_master_rvalid,
	input			 [1					  		:0]	io_master_rresp,
	input			 [DATA_WIDTH * 2-1:0] io_master_rdata,
	input			 											io_master_rlast,
	input			 [3					  		:0]	io_master_rid,

  output                     io_slave_awready,                       
  input                      io_slave_awvalid,
  input [ADDR_WIDTH-1    :0] io_slave_awaddr,
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
  input [ADDR_WIDTH-1    :0] io_slave_araddr,
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

parameter [2:0] IDLE = 3'b0, CLINT = 3'b001, EXU_READ = 3'b010,
								EXU_WRITE = 3'b011, IFU_READ = 3'b100;
reg [2:0] state, next;
always @(posedge clock) begin
  if (reset) 
    state <= IDLE;
  else 
    state <= next;
end

wire [DATA_WIDTH-1:0] clint_addr_min;
wire [DATA_WIDTH-1:0] clint_addr_max;
assign clint_addr_min = 32'h0200_0000;
assign clint_addr_max = 32'h0200_ffff;

wire	 is_clint;
assign is_clint  = (clint_addr_min <= from_dsram_araddr) 
								&& (clint_addr_max >= from_dsram_araddr);

always @(is_clint or from_dsram_awvalid or from_dsram_arvalid or from_isram_arvalid or exu_done or ifu_done) begin
	next = IDLE;
	case (state) 
		IDLE: 
			if (from_isram_arvalid) 
				next = IFU_READ;
			else if (from_dsram_arvalid && is_clint)
				next = CLINT;
			else if (from_dsram_arvalid) 
				next = EXU_READ;
			else if (from_dsram_awvalid) 
				next = EXU_WRITE;
			else
			  next = IDLE;

		IFU_READ:
			if (ifu_done)
				next = IDLE;
			else
				next = IFU_READ;

		CLINT:
			if (exu_done[0])
				next = IDLE;
			else
				next = CLINT;

		EXU_READ:
			if (exu_done[0])
				next = IDLE;
			else
				next = EXU_READ;

		EXU_WRITE:
			if (exu_done[1])
				next = IDLE;
			else
				next = EXU_WRITE;

		default: ;
	endcase
end

always @(*) begin
			io_master_arvalid = 0;
			io_master_araddr  = 0;
			io_master_arid = 0;
			io_master_arlen = 0;
			io_master_arsize = 0;
			io_master_arburst = 0;
			io_master_rready = 0;

			io_master_awvalid = 0;
			io_master_awaddr  = 0;
			io_master_awid = 0;
			io_master_awlen = 0;
			io_master_awsize = 0;
			io_master_awburst = 0;
			
			io_master_wvalid = 0;
			io_master_wdata = 0;
			io_master_wstrb = 0;
			io_master_wlast = 0;
			io_master_bready = 0;

			to_clint_arvalid = 0;
			to_clint_araddr  = 0;
			to_clint_arid = 0;
			to_clint_arlen = 0;
			to_clint_arsize = 0;
			to_clint_arburst = 0;
			to_clint_rready = 0;

			to_dsram_awready = 0;
			to_dsram_wready = 0;
			to_dsram_bvalid = 0;
			to_dsram_bresp = 0;
			to_dsram_bid = 0;

			to_dsram_arready = 0;
			to_dsram_rvalid = 0;
			to_dsram_rresp = 0;
			to_dsram_rdata = 0;
			to_dsram_rlast = 0;
			to_dsram_rid = 0;

			to_isram_arready = 0;
			to_isram_rvalid = 0;
			to_isram_rresp = 0;
			to_isram_rdata = 0;
			to_isram_rlast = 0;
			to_isram_rid = 0;
  case (next)
		IDLE: ;
		IFU_READ: begin
			to_isram_arready = io_master_arready;

			io_master_arvalid = from_isram_arvalid;
			io_master_araddr  = from_isram_araddr;
			io_master_arid = from_isram_arid;
			io_master_arlen = from_isram_arlen;
			io_master_arsize = from_isram_arsize;
			io_master_arburst = from_isram_arburst;
			
			io_master_rready = from_isram_rready;

			to_isram_rvalid = io_master_rvalid;
			to_isram_rresp = io_master_rresp;
			to_isram_rdata = io_master_rdata;
			to_isram_rlast = io_master_rlast;
			to_isram_rid = io_master_rid;
			end
		EXU_READ: begin
			to_dsram_arready = io_master_arready;

			io_master_arvalid = from_dsram_arvalid;
			io_master_araddr  = from_dsram_araddr;
			io_master_arid = from_dsram_arid;
			io_master_arlen = from_dsram_arlen;
			io_master_arsize = from_dsram_arsize;
			io_master_arburst = from_dsram_arburst;
			
			io_master_rready = from_dsram_rready;

			to_dsram_rvalid = io_master_rvalid;
			to_dsram_rresp = io_master_rresp;
			to_dsram_rdata = io_master_rdata;
			to_dsram_rlast = io_master_rlast;
			to_dsram_rid = io_master_rid;
			end
		EXU_WRITE: begin
			to_dsram_awready = io_master_awready;
			
			io_master_awvalid = from_dsram_awvalid;
			io_master_awaddr = from_dsram_awaddr;
			io_master_awid = from_dsram_awid;
			io_master_awlen = from_dsram_awlen;
			io_master_awsize = from_dsram_awsize;
			io_master_awburst = from_dsram_awburst;

			to_dsram_wready = io_master_wready;
			
			io_master_wvalid = from_dsram_wvalid;
			io_master_wdata = from_dsram_wdata;
			io_master_wstrb = from_dsram_wstrb;
			io_master_wlast = from_dsram_wlast;

			io_master_bready = from_dsram_bready;
			to_dsram_bvalid = io_master_bvalid;
			to_dsram_bresp = io_master_bresp;
			to_dsram_bid = io_master_bid;
			end
			
		/*
		DSRAM_READ: begin
			to_dsram_arready = io_master_arready;
			io_master_arvalid = from_dsram_arvalid;
			io_master_araddr = from_dsram_araddr;
			io_master_arid = from_dsram_arid;
			io_master_arlen = from_dsram_arlen;
			io_master_arsize = from_dsram_arsize;
			io_master_arburst = from_dsram_arburst;

			io_master_rready = from_dsram_rready;
			to_dsram_rvalid = io_master_rvalid;
			to_dsram_rresp = io_master_rresp;
			to_dsram_rdata = io_master_rdata;
			to_dsram_rlast = io_master_rlast;
			to_dsram_rid = io_master_rid;
			end
		*/
		CLINT: begin
			to_dsram_arready = from_clint_arready;

			to_clint_arvalid = from_dsram_arvalid;
			to_clint_araddr  = from_dsram_araddr;
			to_clint_arid = from_dsram_arid;
			to_clint_arlen = from_dsram_arlen;
			to_clint_arsize = from_dsram_arsize;
			to_clint_arburst = from_dsram_arburst;
			
			to_clint_rready = from_dsram_rready;

			to_dsram_rvalid = from_clint_rvalid;
			to_dsram_rresp = from_clint_rresp;
			to_dsram_rdata = from_clint_rdata;
			to_dsram_rlast = from_clint_rlast;
			to_dsram_rid = from_clint_rid;
			end
		default:;
	endcase
end

endmodule
