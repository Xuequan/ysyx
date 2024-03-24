// IFU 模块
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [DATA_WIDTH-1:0]  inst_i,  // from inst ram

	// from IDU
	input 								  inst_jal_jalr, 
	// from EXU
	input [DATA_WIDTH-1:0]  nextpc_from_jal_jalr,

	output reg 							valid,						
	output [DATA_WIDTH-1:0] pc,  // to IDU
	output [ADDR_WIDTH-1:0] addr    // to inst ram
);

reg [ADDR_WIDTH-1:0] next_pc; 

parameter [1:0] IDLE = 2'b00,
								NORMAL = 2'b01,
								JUMP   = 2'b10;
reg [1:0] state, next_state;
 
always @(posedge clk) begin
	if (rst) 
		state <= IDLE;
	else
		state <= next_state;
end
always @(state or inst_jal_jalr) begin
	next_state = NORMAL;
	case (state)
		IDLE: if (inst_jal_jalr) 
						next_state = JUMP;
					else
						next_state = NORMAL;
		NORMAL: if (inst_jal_jalr)
						next_state = JUMP;
					else
						next_state = NORMAL;
		JUMP: if (inst_jal_jalr)
						next_state = JUMP;
					else
						next_state = NORMAL;
		default:;
	endcase
end

always @(posedge clk) begin
	if (rst) 
		next_pc <= ADDR_WIDTH'('h8000_0000);
	else begin
		next_pc <= ADDR_WIDTH'('h8000_0000);
		case (next_state)
			IDLE:;
			NORMAL: 
				next_pc <= next_pc + ADDR_WIDTH'('h4);
			JUMP:
				next_pc <= nextpc_from_jal_jalr;
			default:;
		endcase
	end
end	
						
			
/*
always @(posedge clk) begin
	if (rst) 
		next_pc <= ADDR_WIDTH'('h8000_0000);
	else if (inst_jal_jalr)
		next_pc <= nextpc_from_jal_jalr;
	else
		next_pc <= next_pc + ADDR_WIDTH'('h4);
end
*/

ysyx_23060208_PC #(.ADDR_WIDTH(ADDR_WIDTH)) PC_i0(
	.clk(clk),
	.rst(rst),
	.wen(1'b1),
	.next_pc(next_pc),
	.pc(pc)
);

assign valid = 1'b1;
assign addr = next_pc;

endmodule
