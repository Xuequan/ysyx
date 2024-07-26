// "实现一个AXI-Lite接口的 CLINT 模块"
module ysyx_23060208_clint
	#(DATA_WIDTH = 32) (
	input clock,
	input reset,

	// 读请求通道
	input  [DATA_WIDTH-1:0] clint_araddr,
	input										clint_arvalid,
	input	[3						:0] clint_arid,
	input [7						:0] clint_arlen,
	input [2						:0] clint_arsize,
	input [1						:0] clint_arburst,
	output									clint_arready,

	// 读响应通道
	output 										clint_rvalid,
	output [DATA_WIDTH*2-1:0] clint_rdata,
	output [1					  	:0]	clint_rresp,
	output 										clint_rlast,
	output [3							:0]	clint_rid,
	input											clint_rready
);

reg [63:0] mtime_r;
always @(posedge clock) begin
	if (reset) 
		mtime_r <= 0;
	else 
		mtime_r <= mtime_r + 64'b1;
end

// read FSM
parameter [2:0] IDLE = 3'b000, WAIT_ARVA = 3'b001, SHAKED_AR = 3'b010,
                WAIT_RREADY = 3'b011, SHAKED_R = 3'b100;
reg [2:0] state, next;
always @(posedge clock) begin
  if (reset) 
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
always @(posedge clock) begin
	if (reset)
		arready_r <= 0;
	else if (next == IDLE || next == WAIT_ARVA || next == SHAKED_R)
		arready_r <= 1'b1;
	else
		arready_r <= 1'b0;   // 等一个读数据通道完成后，才开始另一个读
end

reg [3:0] rid_r;
always @(posedge clock) begin
	if (reset) 
		rid_r <= 4'b0;
	else if (next == SHAKED_AR)
		rid_r <= clint_arid;
end

assign clint_rid = rid_r;

reg rvalid_r;
assign clint_rvalid = rvalid_r;

reg [1:0] rresp_r;
assign clint_rresp = rresp_r;
reg rlast_r;
assign clint_rlast = rlast_r;

always @(posedge clock) begin
	if (reset)
		rvalid_r <= 0;
	else if (next == SHAKED_AR || next == WAIT_RREADY) begin
		rvalid_r <= 1'b1;
		rresp_r <= 0;
		rlast_r <= 1'b1;
	end
	else begin 
		rvalid_r <= 0;
		rresp_r <= 0;
		rlast_r <= 0;
	end
end

reg [DATA_WIDTH*2-1:0] clint_rdata_r;
assign clint_rdata = clint_rdata_r;
always @(posedge clock) begin
	if (reset)
		clint_rdata_r <= 0;
	else if (next == SHAKED_AR) begin
		$display("clint");
		clint_rdata_r <= mtime_r;
	end
end

endmodule
