// 本模块模拟 isram
module ysyx_23060208_isram
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,
	input 									ifu_allowin,
	// 读请求
	input [DATA_WIDTH-1:0]  isram_araddr,
	input										isram_arvalid,
	output								  isram_arready,
	// 读响应
	output [1						:0] isram_rresp,
	output                  isram_rvalid,
	output [DATA_WIDTH-1:0] isram_rdata,
	input										isram_rready
);

import "DPI-C" function int isram_read(input int raddr);


/* read from isram */
// read FSM
parameter [2:0] IDLE = 3'b000, WAIT_ARVALID = 3'b001, SHAKED_AR = 3'b010,
                WAIT_RREADY = 3'b011, SHAKED_R = 3'b100;
reg [2:0] state, next;
always @(posedge clk) begin
  if (rst) 
    state <= IDLE;
  else 
    state <= next;
end

always @(state or isram_arvalid or isram_arready or isram_rvalid or isram_rready) begin
  next = IDLE;
  case (state)
    IDLE: 
      if (!isram_arready) 
        next = IDLE;
      else if (!isram_arvalid)
        next = WAIT_ARVALID;
      else 
        next = SHAKED_AR;
    WAIT_ARVALID:
      if (isram_arvalid)
        next = SHAKED_AR;
      else
        next = WAIT_ARVALID;
    SHAKED_AR:
      if (!isram_rvalid)
        next = SHAKED_AR;
      else if (!isram_rready)
        next = WAIT_RREADY;
      else 
        next = SHAKED_R;
    WAIT_RREADY:
      if (isram_rready)
        next = SHAKED_R;
      else 
        next = WAIT_RREADY;
    SHAKED_R:
      if (!isram_arready)
        next = IDLE;
      else if (!isram_arvalid)
        next = WAIT_ARVALID;
      else 
        next = SHAKED_AR;
    default: ;
  endcase 
end

reg arready_r;
assign isram_arready = arready_r;
always @(posedge clk) begin
  if (rst)
    arready_r <= 0;
  else if (next == IDLE || next == WAIT_ARVALID || next == SHAKED_R)
    arready_r <= 1'b1;
  else
    arready_r <= 1'b0;   // 等一个读数据通道完成后，才开始另一个读
end

reg rvalid_r;
assign isram_rvalid = rvalid_r;
always @(posedge clk) begin
  if (rst)
    rvalid_r <= 0;
  else if (next == SHAKED_AR || next == WAIT_RREADY)
    rvalid_r <= 1'b1;
  else 
    rvalid_r <= 0;
end

/*
reg [DATA_WIDTH-1:0] araddr_r;
wire [DATA_WIDTH-1:0] raddr;
assign raddr = (araddr_r == 0) ? 32'h8000_0000 : araddr_r;
always @(posedge clk) begin
  if (rst)
    araddr_r <= 0;
  else if (next == SHAKED_AR)
    araddr_r <= isram_araddr;
end
*/
reg [DATA_WIDTH-1:0] rdata_r;
assign isram_rdata = rdata_r;
always @(posedge clk) begin
	if (rst) 
		rdata_r <= 0;
	//else if (next == SHAKED_R) 
  else if (next == SHAKED_AR)
		rdata_r <= isram_read(isram_araddr);
end

endmodule
