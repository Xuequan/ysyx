// "实现一个AXI-Lite接口的 CLINT 模块"
module ysyx_23060208_clint
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,

	// 读请求通道
	input  [DATA_WIDTH-1:0] clint_araddr,
	input										clint_arvalid,
	output									clint_arready,

	// 读响应通道
	output [DATA_WIDTH-1:0] clint_rdata,
	output [1					  :0]	clint_rresp,
	output 									clint_rvalid,
	input										clint_rready
);

reg [63:0] mtime_r;
always @(posedge clk) begin
	if (rst) 
		mtime_r <= 0;
	else 
		mtime_r <= mtime_r + 64'b1;
end

import "DPI-C" function int dsram_read(input int dsram_araddr);

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

always @(state or clint_arvalid or clint_arready or clint_rvalid or clint_rready) begin
  next = IDLE;
  case (state)
    IDLE: 
      if (!clint_arready) 
        next = IDLE;
      else if (!clint_arvalid)
        next = WAIT_ARVA;
      else 
        next = SHAKED_AR;
    WAIT_ARVA:
      if (clint_arvalid)
        next = SHAKED_AR;
      else
        next = WAIT_ARVA;
    SHAKED_AR:
      if (!clint_rvalid)
        next = SHAKED_AR;
      else if (!clint_rready)
        next = WAIT_RREADY;
      else 
        next = SHAKED_R;
    WAIT_RREADY:
      if (clint_rready)
        next = SHAKED_R;
      else 
        next = WAIT_RREADY;
    SHAKED_R:
      if (!clint_arready)
        next = IDLE;
      else if (!clint_arvalid)
        next = WAIT_ARVA;
      else 
        next = SHAKED_AR;
    default: ;
  endcase 
end
reg arready_r;
assign clint_arready = arready_r;
always @(posedge clk) begin
	if (rst)
		arready_r <= 0;
	else if (next == IDLE || next == WAIT_ARVA || next == SHAKED_R)
		arready_r <= 1'b1;
	else
		arready_r <= 1'b0;   // 等一个读数据通道完成后，才开始另一个读
end

reg rvalid_r;
assign clint_rvalid = rvalid_r;
always @(posedge clk) begin
	if (rst)
		rvalid_r <= 0;
	else if (next == SHAKED_AR || next == WAIT_RREADY)
		rvalid_r <= 1'b1;
	else 
		rvalid_r <= 0;
end

wire [DATA_WIDTH-1:0] clint_addr;
assign clint_addr = 32'ha000_0000 + 32'h0000_048;

reg [DATA_WIDTH-1:0] clint_rdata_r;
assign clint_rdata = clint_rdata_r;
always @(posedge clk) begin
	if (rst)
		clint_rdata_r <= 0;
	else if (next == SHAKED_AR && clint_araddr == clint_addr) begin
		//clint_rdata_r <= dsram_read(clint_araddr);
		$display("clint");
		clint_rdata_r <= 32'b1;
	end
	else if (next == SHAKED_AR && clint_araddr == clint_addr + 4)
		//clint_rdata_r <= dsram_read(clint_araddr);
		//clint_rdata_r <= mtime_r[63:32];
		clint_rdata_r <= 32'b0;
	/*
	else if (next == SHAKED_AR && clint_araddr == clint_addr) begin
		//clint_rdata_r <= dsram_read(clint_araddr);
		//clint_rdata_r <= mtime_r[31:0];
		$display("clint");
		clint_rdata_r <= 32'b1;
	end
	else if (next == SHAKED_AR && clint_araddr == clint_addr + 4)
		//clint_rdata_r <= dsram_read(clint_araddr);
		//clint_rdata_r <= mtime_r[63:32];
		clint_rdata_r <= 32'b0;
	*/
end

endmodule
