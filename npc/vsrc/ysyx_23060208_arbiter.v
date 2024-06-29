module ysyx_23060208_arbiter
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	
	input 								 ifu_done,
	input [1					 :0] exu_done,
	//output [2					 :0] grant,

	// 写地址通道:
	      // from master
	input [ADDR_WIDTH-1:0] dsram_awaddr_i,
	input									 dsram_awvalid_i,
				// to slave
	output reg [ADDR_WIDTH-1:0] dsram_awaddr_o,
	output reg									dsram_awvalid_o,
				// from slave
	input								   dsram_awready_i,
				// to master
	output reg								 dsram_awready_o,
	
	// 写数据通道
	  // from master
	input [DATA_WIDTH-1:0] dsram_wdata_i,
	input [2					 :0] dsram_wstrb_i,
	input									 dsram_wvalid_i,
		// to slave
	output reg [DATA_WIDTH-1:0] dsram_wdata_o,
	output reg [2					 :0] dsram_wstrb_o,
	output reg									 dsram_wvalid_o,
		// from slave
	input								 dsram_wready_i,
		// to master
	output reg								 dsram_wready_o,

	// 写响应
	  // from slave
	input [1					 :0] dsram_bresp_i,
	input								   dsram_bvalid_i,
	  // to master
	output reg [1					 :0] dsram_bresp_o,
	output reg								 dsram_bvalid_o,
		// from master
	input									 dsram_bready_i,
		// to slave
	output reg									 dsram_bready_o,

	// 读请求通道
	  // from master
	input  [ADDR_WIDTH-1:0] dsram_araddr_i,
	input										dsram_arvalid_i,
		// to slave
	output reg  [ADDR_WIDTH-1:0] dsram_araddr_o,
	output reg										dsram_arvalid_o,
		// from slave
	input									dsram_arready_i,
		// to master
	output reg									dsram_arready_o,

	// 读响应通道
	  // from slave
	input [DATA_WIDTH-1:0]  dsram_rdata_i,
	input [1					  :0]	dsram_rresp_i,
	input 									dsram_rvalid_i,
		// to master
	output reg [DATA_WIDTH-1:0] dsram_rdata_o,
	output reg [1					  :0]	dsram_rresp_o,
	output reg 									dsram_rvalid_o,
		// from master
	input										dsram_rready_i,
		// to slave
	output reg										dsram_rready_o,
	
	// isram
	// 读请求通道
	  // from master
	input  [ADDR_WIDTH-1:0] isram_araddr_i,
	input										isram_arvalid_i,
		// to slave
	output reg  [ADDR_WIDTH-1:0] isram_araddr_o,
	output reg										isram_arvalid_o,
		// from slave
	input									isram_arready_i,
		// to master
	output reg									isram_arready_o,

	// 读响应通道
	  // from slave
	input [DATA_WIDTH-1:0]  isram_rdata_i,
	input [1					  :0]	isram_rresp_i,
	input 									isram_rvalid_i,
		// to master
	output reg [DATA_WIDTH-1:0] isram_rdata_o,
	output reg [1					  :0]	isram_rresp_o,
	output reg 									isram_rvalid_o,
		// from master
	input										isram_rready_i,
		// to slave
	output reg										isram_rready_o
);

parameter [1:0] IDLE = 2'b0, GRANT_IFU = 2'b01, 
								GRANT_EXU_READ = 2'b10, GRANT_EXU_WRITE = 2'b11;
reg [1:0] state, next;
always @(posedge clk) begin
  if (rst) 
    state <= IDLE;
  else 
    state <= next;
end

always @(isram_arvalid_i or dsram_arvalid_i or dsram_awvalid_i or exu_done or ifu_done) begin
	next = IDLE;
	case (state) 
		IDLE: 
			if (isram_arvalid_i) 
				next = GRANT_IFU;
			else if (dsram_arvalid_i)
				next = GRANT_EXU_READ;
			else if (dsram_awvalid_i)
				next = GRANT_EXU_WRITE;
			else
			  next = IDLE;

		GRANT_IFU:
			if (ifu_done)
				next = IDLE;
			else
				next = GRANT_IFU;

		GRANT_EXU_READ:
			if (exu_done[0])
				next = IDLE;
			else
				next = GRANT_EXU_READ;

		GRANT_EXU_WRITE:
			if (exu_done[1])
				next = IDLE;
			else
				next = GRANT_EXU_WRITE;

	endcase
end

/*
reg [2:0] grant_r;
assign grant = grant_r;
always @(posedge clk) begin
	if (rst) grant_r <= 0;
	else if (next == GRANT_IFU) 
		grant_r <= 3'b001;	
	else if (next == GRANT_EXU_READ)
		grant_r <= 3'b010;	
	else if (next == GRANT_EXU_WRITE)
		grant_r <= 3'b100;	
	else 
		grant_r <= 0;
end
*/
always @(*) begin
	dsram_awvalid_o = 0;
	dsram_awaddr_o = 0;
	dsram_awready_o = 0;
	
	dsram_wdata_o = 0;
	dsram_wstrb_o = 0;
	dsram_wvalid_o = 0;
	dsram_wready_o = 0;

	dsram_bresp_o = 0;
	dsram_bvalid_o = 0;
	dsram_bready_o = 0;

	dsram_arvalid_o = 0;
	dsram_araddr_o = 0;
	dsram_arready_o = 0;

	dsram_rdata_o = 0;
	dsram_rresp_o = 0;
	dsram_rvalid_o = 0;
	dsram_rready_o = 0;

	isram_arvalid_o = 0;
	isram_araddr_o = 0;
	isram_arready_o = 0;

	isram_rdata_o = 0;
	isram_rresp_o = 0;
	isram_rvalid_o = 0;
	isram_rready_o = 0;
	
  case (next)
		IDLE: ;
		GRANT_IFU: begin
			isram_arvalid_o = isram_arvalid_i;
			isram_araddr_o = isram_araddr_i;
			isram_arready_o = isram_arready_i;

			isram_rdata_o = isram_rdata_i;
			isram_rresp_o = isram_rresp_i;
			isram_rvalid_o = isram_rvalid_i;
			isram_rready_o = isram_rready_i;
			end
		GRANT_EXU_READ: begin
			dsram_arvalid_o = dsram_arvalid_i;
			dsram_araddr_o = dsram_araddr_i;
			dsram_arready_o = dsram_arready_i;

			dsram_rdata_o = dsram_rdata_i;
			dsram_rresp_o = dsram_rresp_i;
			dsram_rvalid_o = dsram_rvalid_i;
			dsram_rready_o = dsram_rready_i;
			end
		GRANT_EXU_WRITE: begin
			dsram_awvalid_o = dsram_awvalid_i;
			dsram_awaddr_o = dsram_awaddr_i;
			dsram_awready_o = dsram_awready_i;

			dsram_wdata_o = dsram_wdata_i;
			dsram_wstrb_o = dsram_wstrb_i;
			dsram_wvalid_o = dsram_wvalid_i;
			dsram_wready_o = dsram_wready_i;

			dsram_bresp_o = dsram_bresp_i;
			dsram_bvalid_o = dsram_bvalid_i;
			dsram_bready_o = dsram_bready_i;
			end
	endcase
end

endmodule
