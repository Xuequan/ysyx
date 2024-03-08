// 仅支持有符号数，若想支持无符号数，则需要修改
// 对于 op 为比较大小 src1 < src2 , return result == 1; else return 0
// 对于 op 为判断是否相等，相等 return 1; 
module ysyx_23060208_alu 
	#(parameter DATA_WIDTH = 32) (
	
	output [DATA_WIDTH-1:0] result,
	output overflow,
	input [2:0] op,
	input [DATA_WIDTH-1:0] src1,
	input [DATA_WIDTH-1:0] src2
);
	
	wire [DATA_WIDTH-1:0] add_sub_result;
	wire add_sub_op;
	wire add_sub_ow;
	wire [DATA_WIDTH-1:0] not_result;
	wire [DATA_WIDTH-1:0] and_result;
	wire [DATA_WIDTH-1:0] or_result;
	wire [DATA_WIDTH-1:0] xor_result;
	wire compare_result;
	wire equal_result;
	wire less;

 	// not a
	assign not_result = ~src1;
 	// and
 	assign and_result = src1 & src2;
	// or
 	assign or_result = src1 | src2;
	// xor
 	assign xor_result = src1 ^ src2;
 
	assign add_sub_op = (op == 3'b000) ? 1'b0 : 1'b1;	
	ysyx_23060208_adder #(.DATA_WIDTH(DATA_WIDTH)) adder0(add_sub_result, add_sub_ow, src1, src2, add_sub_op); 

	// 先判断比较
	// Less = out_s[n-1] ^ add_sub_ow
	assign less = add_sub_result[DATA_WIDTH-1] ^ add_sub_ow;
	assign compare_result = less ? 1 : 0;

	assign equal_result = ~(|add_sub_result);

	assign overflow = add_sub_ow;

	assign result = ({DATA_WIDTH{op == 3'b000}} & add_sub_result) |
								 ({DATA_WIDTH{op == 3'b001}} & add_sub_result) |
								 ({DATA_WIDTH{op == 3'b010}} & not_result) |
								 ({DATA_WIDTH{op == 3'b011}} & and_result) |
								 ({DATA_WIDTH{op == 3'b100}} & or_result) |
								 ({DATA_WIDTH{op == 3'b101}} & xor_result) |
								 ({DATA_WIDTH{op == 3'b110}} & {DATA_WIDTH{compare_result}}) |
								 ({DATA_WIDTH{op == 3'b111}} & {DATA_WIDTH{equal_result}});
	
	
endmodule
