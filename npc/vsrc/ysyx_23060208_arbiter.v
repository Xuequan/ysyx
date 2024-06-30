module ysyx_23060208_arbiter
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	
	input 								 ifu_done,
	input [1					 :0] exu_done,

	// ===================================================
	// 写地址通道:
	      // from master
	input [ADDR_WIDTH-1			:0] dsram_awaddr_i,
	input									      dsram_awvalid_i,
				// to slave
	output reg [ADDR_WIDTH-1:0] dsram_awaddr_o,
	output reg									dsram_awvalid_o,
				// from slave
	input								   			dsram_awready_i,
				// to master
	output reg								  dsram_awready_o,
	
	// 写数据通道
	  // from master
	input [DATA_WIDTH-1			:0] dsram_wdata_i,
	input [2					 			:0] dsram_wstrb_i,
	input									 			dsram_wvalid_i,
		// to slave
	output reg [DATA_WIDTH-1:0] dsram_wdata_o,
	output reg [2					  :0] dsram_wstrb_o,
	output reg									dsram_wvalid_o,
		// from slave
	input								 				dsram_wready_i,
		// to master
	output reg								  dsram_wready_o,

	// 写响应
	  // from slave
	input [1					 			:0] dsram_bresp_i,
	input								   			dsram_bvalid_i,
	  // to master
	output reg [1					  :0] dsram_bresp_o,
	output reg								  dsram_bvalid_o,
		// from master
	input									      dsram_bready_i,
		// to slave
	output reg									dsram_bready_o,
	// ===================================================

	// ===================================================
	/* connect with uart, start  */
	// EXU 发起的写，经 xbar 转给了 uart
	// 写地址通道:
	output reg [ADDR_WIDTH-1:0] uart_awaddr_o,
	output reg									uart_awvalid_o,
	input								   			uart_awready_i,
	// 写数据通道
	output reg [DATA_WIDTH-1:0] uart_wdata_o,
	output reg [2					 :0]  uart_wstrb_o,
	output reg									uart_wvalid_o,
	input								 				uart_wready_i,
	// 写响应
	input [1					 :0] 			uart_bresp_i,
	input								   			uart_bvalid_i,
	output reg									uart_bready_o,
	/* connect with uart, end  */
	// ===================================================

	// ===================================================
	/* connect clint */
	// 读请求通道
	  // from master
	//input  [ADDR_WIDTH-1		 :0] dsram_araddr_i,
	//input												 dsram_arvalid_i,
		// to clint
	output reg  [ADDR_WIDTH-1:0] clint_araddr_o,
	output reg									 clint_arvalid_o,
		// from clint
	input												 clint_arready_i,
		// to master
	//output reg									 dsram_arready_o,

	// 读响应通道
	  // from slave
	input [DATA_WIDTH-1			 :0] clint_rdata_i,
	input [1					  		 :0] clint_rresp_i,
	input 											 clint_rvalid_i,
		// to master
	/*
	output reg [DATA_WIDTH-1 :0] dsram_rdata_o,
	output reg [1					   :0] dsram_rresp_o,
	output reg 									 dsram_rvalid_o,
	*/
		// from master
	//input										     dsram_rready_i,
		// to slave
	output reg									 clint_rready_o,
	// ===================================================

	// ===================================================
	// 读请求通道
	  // from master
	input  [ADDR_WIDTH-1		 :0] dsram_araddr_i,
	input												 dsram_arvalid_i,
		// to slave
	output reg  [ADDR_WIDTH-1:0] dsram_araddr_o,
	output reg									 dsram_arvalid_o,
		// from slave
	input												 dsram_arready_i,
		// to master
	output reg									 dsram_arready_o,

	// 读响应通道
	  // from slave
	input [DATA_WIDTH-1			 :0] dsram_rdata_i,
	input [1					  		 :0] dsram_rresp_i,
	input 											 dsram_rvalid_i,
		// to master
	output reg [DATA_WIDTH-1 :0] dsram_rdata_o,
	output reg [1					   :0] dsram_rresp_o,
	output reg 									 dsram_rvalid_o,
		// from master
	input										     dsram_rready_i,
		// to slave
	output reg									 dsram_rready_o,
	// ===================================================
	
	// ===================================================
	// isram
	// 读请求通道
	  // from master
	input  [ADDR_WIDTH-1		 :0] isram_araddr_i,
	input										 		 isram_arvalid_i,
		// to slave
	output reg  [ADDR_WIDTH-1:0] isram_araddr_o,
	output reg									 isram_arvalid_o,
		// from slave
	input												 isram_arready_i,
		// to master
	output reg									 isram_arready_o,

	// 读响应通道
	  // from slave
	input [DATA_WIDTH-1			 :0] isram_rdata_i,
	input [1					  		 :0] isram_rresp_i,
	input 											 isram_rvalid_i,
		// to master
	output reg [DATA_WIDTH-1:0] isram_rdata_o,
	output reg [1					  :0]	isram_rresp_o,
	output reg 									isram_rvalid_o,
		// from master
	input												isram_rready_i,
		// to slave
	output reg									isram_rready_o
);

parameter [2:0] IDLE = 3'b000, ISRAM = 3'b001, UART = 3'b010,
								DSRAM_READ = 3'b011, DSRAM_WRITE = 3'b100,
								CLINT = 3'b101;
reg [2:0] state, next;
always @(posedge clk) begin
  if (rst) 
    state <= IDLE;
  else 
    state <= next;
end

wire [DATA_WIDTH-1:0] serial_addr;
assign serial_addr = 32'ha000_0000 + 32'h0000_3f8;
wire [DATA_WIDTH-1:0] clint_addr;
// clint addr is RTC_ADDR
assign clint_addr = 32'ha000_0000 + 32'h0000_048;

wire	is_serial;
assign is_serial = (serial_addr == dsram_awaddr_i);
wire	is_clint;
assign is_clint  = (clint_addr == dsram_araddr_i) 
								|| (clint_addr + 4 == dsram_araddr_i);

always @(is_clint or isram_arvalid_i or dsram_arvalid_i or dsram_awvalid_i or is_serial or exu_done or ifu_done) begin
	next = IDLE;
	case (state) 
		IDLE: 
			if (isram_arvalid_i) 
				next = ISRAM;
			else if (dsram_arvalid_i)
				next = DSRAM_READ;
			else if (dsram_awvalid_i && is_serial)
				next = UART;
			else if (dsram_arvalid_i && is_clint)
				next = CLINT;
			else if (dsram_awvalid_i)
				next = DSRAM_WRITE;
			else
			  next = IDLE;

		ISRAM:
			if (ifu_done)
				next = IDLE;
			else
				next = ISRAM;

		DSRAM_READ:
			if (exu_done[0])
				next = IDLE;
			else
				next = DSRAM_READ;

		DSRAM_WRITE:
			if (exu_done[1])
				next = IDLE;
			else
				next = DSRAM_WRITE;

		UART:
			if (exu_done[1])
				next = IDLE;
			else
				next = UART;

		CLINT:
			if (exu_done[0])
				next = IDLE;
			else
				next = CLINT;

		default: ;
	endcase
end

always @(*) begin
	/* 这里的输出信号，都是经过 xbar 转换后的信号 */
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

	uart_awvalid_o = 0;
	uart_awaddr_o  = 0;

	uart_wdata_o = 0;
	uart_wstrb_o = 0;
	uart_wvalid_o = 0;

	uart_bready_o = 0;

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
	
	clint_arvalid_o = 0;
	clint_araddr_o = 0;

	clint_rready_o = 0;
	
  case (next)
		IDLE: ;
		ISRAM: begin
			isram_arvalid_o = isram_arvalid_i;
			isram_araddr_o = isram_araddr_i;
			isram_arready_o = isram_arready_i;

			isram_rdata_o = isram_rdata_i;
			isram_rresp_o = isram_rresp_i;
			isram_rvalid_o = isram_rvalid_i;
			isram_rready_o = isram_rready_i;
			end
		DSRAM_READ: begin
			dsram_arvalid_o = dsram_arvalid_i;
			dsram_araddr_o = dsram_araddr_i;
			dsram_arready_o = dsram_arready_i;

			dsram_rdata_o = dsram_rdata_i;
			dsram_rresp_o = dsram_rresp_i;
			dsram_rvalid_o = dsram_rvalid_i;
			dsram_rready_o = dsram_rready_i;
			end
		DSRAM_WRITE: begin
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
		UART: begin
			uart_awvalid_o = dsram_awvalid_i;
			uart_awaddr_o  = dsram_awaddr_i;
			dsram_awready_o = uart_awready_i;

			uart_wdata_o = dsram_wdata_i;
			uart_wstrb_o = dsram_wstrb_i;
			uart_wvalid_o = dsram_wvalid_i;
			dsram_wready_o = uart_wready_i;

			dsram_bresp_o = uart_bresp_i;
			dsram_bvalid_o = uart_bvalid_i;
			uart_bready_o = dsram_bready_i;
			end
		CLINT: begin
			clint_arvalid_o = dsram_arvalid_i;
			clint_araddr_o = dsram_araddr_i;
			dsram_arready_o = clint_arready_i;

			dsram_rdata_o = clint_rdata_i;
			dsram_rresp_o = clint_rresp_i;
			dsram_rvalid_o = clint_rvalid_i;
			clint_rready_o = dsram_rready_i;
			end
		default:;
	endcase
end

endmodule
