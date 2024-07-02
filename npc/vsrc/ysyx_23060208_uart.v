// 本模块用于“实现AXI-Lite接口的UART功能” 而新增的
module ysyx_23060208_uart
	#(DATA_WIDTH = 32) (
	input clock,
	input reset,
	
	// 写地址通道
	input [DATA_WIDTH-1:0] uart_awaddr,
	input									 uart_awvalid,
	output								 uart_awready,
	
	// 写数据通道
	input [DATA_WIDTH-1:0] uart_wdata,
	input [2					 :0] uart_wstrb,
	input									 uart_wvalid,
	output								 uart_wready,

	// 写响应
	output [1					 :0] uart_bresp,
	output								 uart_bvalid,
	input									 uart_bready
);

import "DPI-C" function void dsram_write(
	input int awaddr, input int dsram_wdata, input byte wmask);

/* write to dsram */
parameter [2:0] IDLE_W = 3'b000, WAIT_AWVALID = 3'b001, SHAKED_AW = 3'b010,
                WAIT_WVALID = 3'b011, SHAKED_W = 3'b100, 
                WAIT_BREADY = 3'b101, SHAKED_B = 3'b110;
reg [2:0] state_w, next_w;
always @(posedge clock) begin
  if (reset) 
    state_w <= IDLE_W;
  else 
    state_w <= next_w;
end

always @(state_w or uart_awvalid or uart_awready or uart_wvalid or uart_wready or uart_bvalid or uart_bready) begin
  next_w = IDLE_W;
  case (state_w)
    IDLE_W: 
      if (!uart_awready) 
        next_w = IDLE_W;
      else if (!uart_awvalid)
        next_w = WAIT_AWVALID;
      else 
        next_w = SHAKED_AW;
    WAIT_AWVALID:
      if (!uart_awvalid) 
        next_w = WAIT_AWVALID;
      else
        next_w = SHAKED_AW;
    SHAKED_AW:
      if (!uart_wready)
        next_w = SHAKED_AW;
      else if (!uart_wvalid)
        next_w = WAIT_WVALID;
      else 
        next_w = SHAKED_W;
    WAIT_WVALID:
      if (!uart_wvalid)
        next_w = WAIT_WVALID;
      else 
        next_w = SHAKED_W;
    SHAKED_W:
      if (!uart_bvalid)
        next_w = SHAKED_W;
      else if (!uart_bready)
        next_w = WAIT_BREADY;
      else
        next_w = SHAKED_B;
    WAIT_BREADY:
      if (!uart_bready)
        next_w = WAIT_BREADY;
      else
        next_w = SHAKED_B;
    SHAKED_B:
      if (!uart_awready)
        next_w = IDLE_W;
      else if (!uart_awvalid)
        next_w = WAIT_AWVALID;
      else 
        next_w = SHAKED_AW;
    default: ;
  endcase
end

reg awready_r;
assign uart_awready = awready_r;
always @(posedge clock) begin
  if (reset) awready_r <= 1'b0;
  else if (next_w == IDLE_W || next_w == WAIT_AWVALID 
		|| next_w == SHAKED_B)
    awready_r <= 1'b1;
  else
    awready_r <= 1'b0;
end
reg wready_r;
assign uart_wready = wready_r;
always @(posedge clock) begin
  if (reset) wready_r <= 1'b0;
  else if (next_w == SHAKED_AW || next_w == WAIT_WVALID) 
    wready_r <= 1'b1;
  else
    wready_r <= 1'b0;
end

reg bvalid_r;
assign uart_bvalid = bvalid_r;
always @(posedge clock) begin
  if (reset) bvalid_r <= 1'b0;
  else if (next_w == SHAKED_W || next_w == WAIT_BREADY) 
    bvalid_r <= 1'b1;
  else
    bvalid_r <= 1'b0;
end

reg [DATA_WIDTH-1:0] awaddr_r;
always @(posedge clock) begin
	if (reset) awaddr_r <= 0;
	else if (next_w == SHAKED_AW)
		awaddr_r <= uart_awaddr;
end

wire [7:0] wmask;
assign wmask[7:3] = 5'b0;
assign wmask[2:0] = uart_wstrb;

assign uart_wready = 1'b1;

wire [DATA_WIDTH-1:0] awaddr;
assign awaddr = awaddr_r;
always @(*) begin
	if (next_w == SHAKED_W) begin	 
		//$display("uart");
		dsram_write(awaddr, uart_wdata, wmask);
		//$write(uart_wdata[7:0]);
	end
end

endmodule
