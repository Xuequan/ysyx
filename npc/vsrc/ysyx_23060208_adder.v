// 仅支持有符号数
module ysyx_23060208_adder
	#(parameter DATA_WIDTH = 32)(
	output [DATA_WIDTH-1:0] sum,
	output overflow,
	input [DATA_WIDTH-1:0] a,
	input [DATA_WIDTH-1:0] b,
	input op
);

	wire [DATA_WIDTH-1:0] adder_b;
	wire [DATA_WIDTH-1:0] adder_a;
	
	assign adder_a = a;
	assign adder_b = ({DATA_WIDTH{op}} ^ b) + {{(DATA_WIDTH-1){1'b0}}, op};
	
	assign {overflow, sum} = adder_a + adder_b;
endmodule
	
