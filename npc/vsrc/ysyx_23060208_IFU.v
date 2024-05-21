// IFU 模块
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,

	// from EXU
	input [DATA_WIDTH-1:0]  exu_nextpc,
	input 									exu_nextpc_taken,


	/* connect with isram */
	input [DATA_WIDTH-1:0]  inst_i,  
	output reg 							valid,  
	output [DATA_WIDTH-1:0] nextPC, 

	/* connect with IDU */
	//output [DATA_WIDTH-1:0] pc,  
	output [DATA_WIDTH * 2 - 1:0] ifu_to_idu_data_o,  
	input										idu_to_ifu_ready,
	output 									ifu_to_idu_valid
);

wire [DATA_WIDTH-1:0] pc;
assign nextPC = exu_nextpc_taken ? exu_nextpc :
													pc + 4;

ysyx_23060208_PC #(.DATA_WIDTH(DATA_WIDTH)) PC_i0(
	.clk(clk),
	.rst(rst),
	.wen(1'b1),
	.next_pc(nextPC),
	.pc(pc)
);

assign valid = 1'b1;

/* =================== 异步总线 ==================== */
// ifu_valid 表示当前 IFU 有有效的数据
wire ifu_valid;
assign ifu_valid = 1'b1;

assign ifu_to_idu_valid = ifu_valid;

parameter IDLE = 1'b0, 
					WAIT_READY = 1'b1;

reg state, next;
always @(posedge clk) begin
	if (rst) 
		state <= IDLE;
	else 
		state <= next;
end 

always @(state or idu_to_ifu_ready or ifu_to_idu_valid) begin
	next = 1'b0;
	case (state)
		IDLE: 
			if (ifu_to_idu_valid) next = WAIT_READY;
			else 									next = IDLE;
		WAIT_READY:
			if (idu_to_ifu_ready) next = IDLE;
			else									next = WAIT_READY;
	endcase
end	

// ifu_to_idu_data_r : store the data, which should be sent to IDU
// once idu_to_ifu_ready is received
reg [DATA_WIDTH * 2 - 1:0] ifu_to_idu_data_r;
always @(posedge clk) begin
	if (rst)
		ifu_to_idu_data_r <= 0;
	else begin
		ifu_to_idu_data_r <= 0;
		case (next)
			IDLE: 
				ifu_to_idu_data_r <= {pc, inst_i};
			WAIT_READY:
				ifu_to_idu_data_r <= {pc, inst_i};
		endcase
	end
end
assign ifu_to_idu_data_o = ifu_to_idu_data_r;	
/* ==================== DPI-C ====================== */
export "DPI-C" task get_nextPC;
task get_nextPC (output [DATA_WIDTH-1:0] o);
	o = nextPC;
endtask
endmodule
