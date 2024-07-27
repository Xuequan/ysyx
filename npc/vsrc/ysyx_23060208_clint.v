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

assign clint_rdata = mtime_r;

// read FSM
parameter [1:0] IDLE = 2'b00, 			 SHAKED_AR = 2'b01,
                WAIT_RREADY = 2'b10, SHAKED_R = 2'b11;
reg [1:0] state, next;
always @(posedge clock) begin
  if (reset) 
    state <= IDLE;
  else 
    state <= next;
end

always @(state or clint_arvalid or clint_rready) begin
  next = IDLE;
  case (state)
    IDLE: 
      if (!clint_arvalid) 
        next = IDLE;
      else 
        next = SHAKED_AR;

    SHAKED_AR:
      if (!clint_rready)
        next = WAIT_RREADY;
      else 
        next = SHAKED_R;

    WAIT_RREADY:
      if (clint_rready)
        next = SHAKED_R;
      else 
        next = WAIT_RREADY;

    SHAKED_R:
      next = IDLE;

    default: ;
  endcase 
end
reg arready_r;
assign clint_arready = arready_r;
always @(posedge clock) begin
	if (reset)
		arready_r <= 0;
	else if (state == IDLE)
		arready_r <= 1'b1;
	else
		arready_r <= 1'b0; 
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

assign clint_rresp = 2'b0;
reg rlast_r;
assign clint_rlast = rlast_r;

always @(posedge clock) begin
	if (reset) begin
		rvalid_r <= 1'b0;
		rlast_r <= 1'b0;
	end
	else if (next == SHAKED_AR || next == WAIT_RREADY) begin
		$display("clint");
		rvalid_r <= 1'b1;
		rlast_r <= 1'b1;
	end
	else begin 
		rvalid_r <= 0;
		rlast_r <= 0;
	end
end
endmodule
