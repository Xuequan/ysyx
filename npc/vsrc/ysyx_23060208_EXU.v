// EXU 模块
module ysyx_23060208_EXU
	#(DATA_WIDTH = 32, REG_WIDTH = 5) (
	input clk,
	input rst,

	input [1           :0] uncond_jump_inst,
	input [DATA_WIDTH-1:0] pc,
	input [DATA_WIDTH-1:0] src1,
	input [DATA_WIDTH-1:0] src2,
	input [REG_WIDTH-1 :0] rd,
	input [17					 :0] op,

	// regfile_mem_mux == 2'b01, need updating regfile
	// == 2'b10, write to mem; 
	// == 2'b00, no need update both
	input [1					 :0] regfile_mem_mux,

	input [DATA_WIDTH-1:0] cond_branch_target,
	input 								 cond_branch_inst,
	input [4					 :0] load_inst,
	//output [DATA_WIDTH-1:0] nextpc_from_jal_jalr,

	// to IFU for gen nextPC
	output [DATA_WIDTH-1:0] exu_nextpc,
	output									exu_nextpc_taken,
	
	// output to register file
	output [DATA_WIDTH-1:0] regfile_wdata,
	output [REG_WIDTH-1 :0] regfile_waddr,
	output								  regfile_wen,

	/* store -- output to sram  */
	input  [2						:0] store_inst,
	output [DATA_WIDTH-1:0] store_address,
	output									store_en,
	input  [DATA_WIDTH-1:0] store_data_raw, 
	output [DATA_WIDTH-1:0] store_data, 
	output [2						:0] store_bytes_num,
		// load
	input  [DATA_WIDTH-1:0] rdata,  // from dsram
	output [DATA_WIDTH-1:0] raddr,
	output									valid,  // to dsram
	
	/* CSR for nextpc   */
	input  [DATA_WIDTH-1:0] csr_nextpc, 
	input 									csr_nextpc_taken,
	
	/* for CSR update, to IDU */
	input [1:0]              csr_inst,
	output  [DATA_WIDTH-1:0] csr_wdata
);

wire [DATA_WIDTH-1:0] alu_result;
wire overflow;

ysyx_23060208_alu alu(
	.alu_result(alu_result),
	.integer_overflow(overflow),
	.alu_op(op),
	.alu_src1(src1),
	.alu_src2(src2)
);

/* ======branch (to_IFU for generate nextPC ================= */
wire cond_branch_taken;
// conditional branch taken
assign cond_branch_taken = cond_branch_inst && alu_result[0];

// 注意：若是 jalr, 还需要将相加得来的地址LSB 设为 0；
wire [DATA_WIDTH-1:0] branch_target;
assign branch_target = 
			({DATA_WIDTH{uncond_jump_inst[0]}} & alu_result)
		| ({DATA_WIDTH{uncond_jump_inst[1]}} & {alu_result[DATA_WIDTH-1:1], 1'b0})
		| ({DATA_WIDTH{cond_branch_taken}} & cond_branch_target);

// 判断是否是 branch
wire branch_taken;
assign branch_taken = |uncond_jump_inst || cond_branch_taken;
/* ========= output to IFU for nextPC generated ========= */
		// final exu_nextpc 
assign exu_nextpc = ({DATA_WIDTH{branch_taken    }} & branch_target) 
									| ({DATA_WIDTH{csr_nextpc_taken}} & csr_nextpc  );
assign exu_nextpc_taken = branch_taken || csr_nextpc_taken;
/* =======store instruction ============================== */
assign store_address = alu_result; 
assign store_en = regfile_mem_mux[1];
assign store_data = store_data_raw; 
assign store_bytes_num = ( {3{store_inst[0]}} & 3'b100 )
											| ( {3{store_inst[1]}} & 3'b010 )
											| ( {3{store_inst[2]}} & 3'b001 );

/* =======load instruction ============================== */
assign valid = |load_inst;
assign raddr = alu_result;
wire [DATA_WIDTH-1:0] load_data;
assign load_data = ({DATA_WIDTH{load_inst[0]}} & rdata)
| ({DATA_WIDTH{load_inst[1]}} & {{16{rdata[15]}}, rdata[15:0]})
| ({DATA_WIDTH{load_inst[2]}} & { 16'b0, rdata[15:0]})
| ({DATA_WIDTH{load_inst[3]}} & {{24{rdata[7]}}, rdata[7:0]})
| ({DATA_WIDTH{load_inst[4]}} & { 24'b0, rdata[7:0]});

/* ============ to regfile ============================== */
// 若是 jal, jalr, 那么将 rd <- pc + 4
assign regfile_wdata = |uncond_jump_inst ? pc + 4 : 
											 |load_inst     ? load_data :
											 |csr_inst      ? src2      : // csrrw, csrrs
																				alu_result;

assign regfile_waddr = rd;
	// regfile 写使能
assign regfile_wen = regfile_mem_mux[0];

/* ============= to CSR ================================= */
assign csr_wdata = csr_inst[0] ? src1 : alu_result;  // only for csrrs
/* =============== DPI-C ========================= */
export "DPI-C" task update_regfile_no;
task update_regfile_no (output [REG_WIDTH-1:0] reg_no);
	reg_no = regfile_wen ? regfile_waddr : 'b0;
endtask

export "DPI-C" task update_regfile_data;
task update_regfile_data (output [DATA_WIDTH-1:0] din);
	din    = regfile_wdata;
endtask

endmodule

