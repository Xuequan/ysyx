// 本模块模拟 dsram
module ysyx_23060208_dsram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	
	// 写地址通道
	input [ADDR_WIDTH-1:0] dsram_awaddr,
	input									 dsram_awvalid,
	output								 dsram_awready,
	
	// 写数据通道
	input [DATA_WIDTH-1:0] dsram_wdata,
	input [2					 :0] dsram_wstrb,
	input									 dsram_wvalid,
	output								 dsram_wready,

	// 写响应
	output [1					 :0] dsram_bresp,
	output								 dsram_bvalid,
	input									 dsram_bready,

	// 读请求通道
	input  [ADDR_WIDTH-1:0] dsram_araddr,
	input										dsram_arvalid,
	output									dsram_arready,

	// 读响应通道
	output [DATA_WIDTH-1:0] dsram_rdata,
	output [1					  :0]	dsram_rresp,
	output 									dsram_rvalid,
	input										dsram_rready
);

import "DPI-C" function int dsram_read(input int dsram_araddr);
import "DPI-C" function void dsram_write(
	input int awaddr, input int dsram_wdata, input byte wmask);

/* write to dsram */
parameter [2:0] IDLE_W = 3'b000, WAIT_AWVALID = 3'b001, SHAKED_AW = 3'b010,
                WAIT_WVALID = 3'b011, SHAKED_W = 3'b100, 
                WAIT_BREADY = 3'b101, SHAKED_B = 3'b110;
reg [2:0] state_w, next_w;
always @(posedge clk) begin
  if (rst) 
    state_w <= IDLE_W;
  else 
    state_w <= next_w;
end

always @(state_w or dsram_awvalid or dsram_awready or dsram_wvalid or dsram_wready or dsram_bvalid or dsram_bready) begin
  next_w = IDLE_W;
  case (state_w)
    IDLE_W: 
      if (!dsram_awready) 
        next_w = IDLE_W;
      else if (!dsram_awvalid)
        next_w = WAIT_AWVALID;
      else 
        next_w = SHAKED_AW;
    WAIT_AWVALID:
      if (!dsram_awvalid) 
        next_w = WAIT_AWVALID;
      else
        next_w = SHAKED_AW;
    SHAKED_AW:
      if (!dsram_wready)
        next_w = SHAKED_AW;
      else if (!dsram_wvalid)
        next_w = WAIT_WVALID;
      else 
        next_w = SHAKED_W;
    WAIT_WVALID:
      if (!dsram_wvalid)
        next_w = WAIT_WVALID;
      else 
        next_w = SHAKED_W;
    SHAKED_W:
      if (!dsram_bvalid)
        next_w = SHAKED_W;
      else if (!dsram_bready)
        next_w = WAIT_BREADY;
      else
        next_w = SHAKED_B;
    WAIT_BREADY:
      if (!dsram_bready)
        next_w = WAIT_BREADY;
      else
        next_w = SHAKED_B;
    SHAKED_B:
      if (!dsram_awready)
        next_w = IDLE_W;
      else if (!dsram_awvalid)
        next_w = WAIT_AWVALID;
      else 
        next_w = SHAKED_AW;
    default: ;
  endcase
end

reg awready_r;
assign dsram_awready = awready_r;
always @(posedge clk) begin
  if (rst) awready_r <= 1'b0;
  else if (next_w == IDLE_W || next_w == WAIT_AWVALID 
		|| next_w == SHAKED_B)
    awready_r <= 1'b1;
  else
    awready_r <= 1'b0;
end
reg wready_r;
assign dsram_wready = wready_r;
always @(posedge clk) begin
  if (rst) wready_r <= 1'b0;
  else if (next_w == SHAKED_AW || next_w == WAIT_WVALID) 
    wready_r <= 1'b1;
  else
    wready_r <= 1'b0;
end

reg bvalid_r;
assign dsram_bvalid = bvalid_r;
always @(posedge clk) begin
  if (rst) bvalid_r <= 1'b0;
  else if (next_w == SHAKED_W || next_w == WAIT_BREADY) 
    bvalid_r <= 1'b1;
  else
    bvalid_r <= 1'b0;
end

reg [DATA_WIDTH-1:0] awaddr_r;
always @(posedge clk) begin
	if (rst) awaddr_r <= 0;
	else if (next_w == SHAKED_AW)
		awaddr_r <= dsram_awaddr;
end

wire [7:0] wmask;
assign wmask[7:3] = 5'b0;
assign wmask[2:0] = dsram_wstrb;

assign dsram_wready = 1'b1;

wire [DATA_WIDTH-1:0] awaddr;
assign awaddr = awaddr_r;
always @(*) begin
	if (next_w == SHAKED_W) begin	 
		dsram_write(awaddr, dsram_wdata, wmask);
	end
end

/* read from dsram */
// read FSM
parameter [2:0] IDLE = 3'b000, WAIT_ARVA = 3'b001, SHAKED_AR = 3'b010,
                WAIT_RREADY = 3'b011, SHAKED_R = 3'b100;
reg [2:0] state, next;
always @(posedge clk) begin
  if (rst) 
    state <= IDLE;
  else 
    state <= next;
end

always @(state or dsram_arvalid or dsram_arready or dsram_rvalid or dsram_rready) begin
  next = IDLE;
  case (state)
    IDLE: 
      if (!dsram_arready) 
        next = IDLE;
      else if (!dsram_arvalid)
        next = WAIT_ARVA;
      else 
        next = SHAKED_AR;
    WAIT_ARVA:
      if (dsram_arvalid)
        next = SHAKED_AR;
      else
        next = WAIT_ARVA;
    SHAKED_AR:
      if (!dsram_rvalid)
        next = SHAKED_AR;
      else if (!dsram_rready)
        next = WAIT_RREADY;
      else 
        next = SHAKED_R;
    WAIT_RREADY:
      if (dsram_rready)
        next = SHAKED_R;
      else 
        next = WAIT_RREADY;
    SHAKED_R:
      if (!dsram_arready)
        next = IDLE;
      else if (!dsram_arvalid)
        next = WAIT_ARVA;
      else 
        next = SHAKED_AR;
    default: ;
  endcase 
end
reg arready_r;
assign dsram_arready = arready_r;
always @(posedge clk) begin
	if (rst)
		arready_r <= 0;
	else if (next == IDLE || next == WAIT_ARVA || next == SHAKED_R)
		arready_r <= 1'b1;
	else
		arready_r <= 1'b0;   // 等一个读数据通道完成后，才开始另一个读
end

reg rvalid_r;
assign dsram_rvalid = rvalid_r;
always @(posedge clk) begin
	if (rst)
		rvalid_r <= 0;
	else if (next == SHAKED_AR || next == WAIT_RREADY)
		rvalid_r <= 1'b1;
	else 
		rvalid_r <= 0;
end

reg [DATA_WIDTH-1:0] dsram_rdata_r;
assign dsram_rdata = dsram_rdata_r;
always @(posedge clk) begin
	if (rst)
		dsram_rdata_r <= 0;
	else if (next == SHAKED_R)
		dsram_rdata_r <= dsram_read(dsram_araddr);
end

endmodule
