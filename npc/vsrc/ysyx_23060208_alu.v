/* note: alu_src1 is from pc, 0, or reg
**       alu_src2 is from imm or reg;
** 1. op_bge, op_bgeu, 
**			����ת���� src1 >= src2), return alu_result = 1; 
**			����ת return alu_result = 0;
** 2. op_beq, op_bne
**			����ת return alu_result = 1; 
**			����ת return alu_result = 0;
** 3. op_blt, op_bltu
**			����ת return alu_result = 1; 
**			����ת return alu_result = 0;
*/ 
module ysyx_23060208_alu(
  input  [17:0] alu_op,
  input  [31:0] alu_src1,
  input  [31:0] alu_src2,
  output [31:0] alu_result,
  output 		integer_overflow
);

wire op_add;   //�ӷ�����
wire op_sub;   //��������
wire op_slt;   //�з��űȽϣ�С����λ
wire op_sltu;  //�޷��űȽϣ�С����λ
wire op_and;   //��λ��
wire op_nor;   //��λ���
wire op_or;    //��λ��
wire op_xor;   //��λ���
wire op_sll;   //�߼�����
wire op_srl;   //�߼�����
wire op_sra;   //��������
wire op_lui;   //���������ڸ߰벿��
wire op_beq;
wire op_bne;
wire op_blt;
wire op_bltu;
wire op_bge;
wire op_bgeu;

// control code decomposition
assign op_add  = alu_op[ 0];
assign op_sub  = alu_op[ 1];
assign op_slt  = alu_op[ 2];
assign op_sltu = alu_op[ 3];
assign op_and  = alu_op[ 4];
assign op_nor  = alu_op[ 5];
assign op_or   = alu_op[ 6];
assign op_xor  = alu_op[ 7];
assign op_sll  = alu_op[ 8];
assign op_srl  = alu_op[ 9];
assign op_sra  = alu_op[10];
assign op_lui  = alu_op[11];
assign op_beq  = alu_op[12];
assign op_bne  = alu_op[13];
assign op_blt  = alu_op[14];
assign op_bltu = alu_op[15];
assign op_bge  = alu_op[16];
assign op_bgeu = alu_op[17];


wire [31:0] add_sub_result; 
wire [31:0] slt_result; 
wire [31:0] sltu_result;
wire [31:0] and_result;
wire [31:0] nor_result;
wire [31:0] or_result;
wire [31:0] xor_result;
wire [31:0] lui_result;
wire [31:0] sll_result; 
wire [63:0] sr64_result; 
wire [31:0] sr_result; 
wire [31:0] beq_result; 
wire [31:0] bne_result; 
wire [31:0] blt_result; 
wire [31:0] bltu_result; 
wire [31:0] bge_result; 
wire [31:0] bgeu_result; 

// 32-bit adder
wire [31:0] adder_a;
wire [31:0] adder_b;
wire        adder_cin;
wire [31:0] adder_result;
wire        adder_cout;

assign adder_a   = alu_src1;
assign adder_b   = (op_sub | op_slt | op_sltu 
									| op_beq | op_bge | op_bne
									| op_bgeu | op_blt | op_bltu) 
										? ~alu_src2 : alu_src2;

assign adder_cin = (op_sub | op_slt | op_sltu 
									| op_beq | op_bne	| op_bge 
									| op_bgeu | op_blt | op_bltu)
									  ? 1'b1      : 1'b0;

assign {adder_cout, adder_result} = adder_a + adder_b + {31'b0, adder_cin};

//���ڼӷ��������������õ������� �����Ӹ����õ�����
//���ڼ����������������õ������� �����������õ�����
assign integer_overflow = (op_add && ~alu_src1[31] && ~alu_src2[31] && adder_result[31]) ||
(op_add && alu_src1[31] && alu_src2[31] && ~adder_result[31]) ||
(op_sub && ~alu_src1[31] && alu_src2[31] && adder_result[31]) ||(op_sub && alu_src1[31] && ~alu_src2[31] && ~adder_result[31]);

// BEQ result
assign beq_result[31:1] = 31'b0;
assign beq_result[0] = (adder_result == 32'b0);
// BNE result
assign bne_result[31:1] = 31'b0;
assign bne_result[0] = (adder_result != 32'b0);
// BLT result
assign blt_result[31:1] = 31'b0;
assign blt_result[0] = (adder_cout == 1'b1) | (adder_cout == 1'b0
																			& adder_result[31] == 1'b1);
// BLTU result
assign bltu_result[31:1] = 31'b0;
assign bltu_result[0] = (adder_cout == 1'b1); 
// BGE result
assign bge_result[31:1] = 31'b0;
assign bge_result[0] = (adder_cout == 1'b0) & (adder_result[31] == 1'b0); 
// BGEU result
assign bgeu_result[31:1] = 31'b0;
assign bgeu_result[0] = (adder_cout == 1'b0);
// ADD, SUB result
assign add_sub_result = adder_result;

// SLT result
assign slt_result[31:1] = 31'b0;
assign slt_result[0]    = (alu_src1[31] & ~alu_src2[31])
          | ((alu_src1[31] ~^ alu_src2[31]) & adder_result[31]);

// SLTU result
assign sltu_result[31:1] = 31'b0;
assign sltu_result[0]    = ~adder_cout;

// bitwise operation
assign and_result = alu_src1 & alu_src2;
assign or_result  = alu_src1 | alu_src2;

assign nor_result = ~or_result;
assign xor_result = alu_src1 ^ alu_src2;
assign lui_result = alu_src2;

// SLL result 
assign sll_result = alu_src1 << alu_src2[4:0];

// SRL, SRA result
assign sr64_result = { {32{op_sra & alu_src1[31]}}, alu_src1[31:0]} >> alu_src2[4:0];

assign sr_result   = sr64_result[31:0];

// final result mux
assign alu_result = ({32{op_add|op_sub}} & add_sub_result)
                  | ({32{op_slt       }} & slt_result)
                  | ({32{op_sltu      }} & sltu_result)
                  | ({32{op_and       }} & and_result)
                  | ({32{op_nor       }} & nor_result)
                  | ({32{op_or        }} & or_result)
                  | ({32{op_xor       }} & xor_result)
                  | ({32{op_lui       }} & lui_result)
                  | ({32{op_sll       }} & sll_result)
                  | ({32{op_srl|op_sra}} & sr_result)
                  | ({32{op_beq       }} & beq_result)
                  | ({32{op_bne       }} & bne_result)
                  | ({32{op_blt       }} & blt_result)
                  | ({32{op_bltu      }} & bltu_result)
                  | ({32{op_bge       }} & bge_result)
                  | ({32{op_bgeu      }} & bgeu_result);


endmodule
