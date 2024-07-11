`include "ysyx_23060208_npc.h"    
module ysyx_23060208_EXU
	#(DATA_WIDTH = 32, REG_WIDTH = 5) (
	input clock,
	input reset,

	/* to IFU (for gen nextPC)    */
	output [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus,
	output									     exu_to_ifu_valid,

	/* update regfile */
	output [DATA_WIDTH-1:0] regfile_wdata,
	output [REG_WIDTH-1 :0] regfile_waddr,
	output								  regfile_wen,

	/* connect with intercom*/
	output [1							:0]	exu_done,

		// 写地址通道
	input										dsram_awready,
	output [DATA_WIDTH-1:0] dsram_awaddr,
	output									dsram_awvalid,
	
	/* 新增 */
	output [3						:0] dsram_awid,
	output [7						:0] dsram_awlen,
	output [2						:0] dsram_awsize,
	output [1						:0] dsram_awburst,

	// 写数据通道
	output 										dsram_wvalid,
	output [DATA_WIDTH*2-1:0] dsram_wdata, 
	output [7							:0] dsram_wstrb,
	output 										dsram_wlast,
	input											dsram_wready,
	// 写响应通道
	input										 dsram_bvalid,
	input  [1						 :0] dsram_bresp,
	input	 [3						 :0] dsram_bid,
	output									 dsram_bready,

	// 读请求通道
	input										dsram_arready,
	output									dsram_arvalid, 
	output [DATA_WIDTH-1:0] dsram_araddr,
	output [3						:0] dsram_arid,
	output [7						:0] dsram_arlen,
	output [2						:0] dsram_arsize,
	output [1						:0] dsram_arburst,
	// 读响应通道
	output									 dsram_rready,
	input										 dsram_rvalid,
	input [DATA_WIDTH*2-1:0] dsram_rdata,  
	input [1:							0] dsram_rresp,
	input										 dsram_rlast,
	input [3:							0] dsram_rid,

	
	/* connect with CSR */
	output [DATA_WIDTH-1:0] csr_wdata,
	output [DATA_WIDTH-1:0] csr_wdata2,
	output [11					:0] csr_waddr,
	output [11					:0] csr_waddr2,

	/* connect with IDU */
	input [`IDU_TO_EXU_ALU_BUS-1:0] idu_to_exu_alu_bus,
	input [`IDU_TO_EXU_BUS-1    :0] idu_to_exu_bus,
	// CSR for nextpc, from IDU
	input [`IDU_TO_EXU_CSR_BUS-1:0] idu_to_exu_csr_bus,

	input 									idu_to_exu_valid,
	
	output									exu_allowin
);
reg [3:0] tik;
always @(posedge clock) begin
	if (reset)
		tik <= 0;
	else
		tik <= (tik == 4'b1111) ? 0 : tik + 1;
end

reg [`IDU_TO_EXU_ALU_BUS-1:0] idu_to_exu_alu_bus_r;
reg [`IDU_TO_EXU_BUS-1    :0] idu_to_exu_bus_r;
reg [`IDU_TO_EXU_CSR_BUS-1:0] idu_to_exu_csr_bus_r;

wire [DATA_WIDTH-1:0] src1;
wire [DATA_WIDTH-1:0] src2;
wire [REG_WIDTH-1 :0] rd;
wire [17					 :0] op;
assign {src1, src2, rd, op} = idu_to_exu_alu_bus_r;

wire [1					 :0] regfile_mem_mux;
wire [2					 :0] store_inst;
wire [4					 :0] load_inst;
wire [DATA_WIDTH-1:0] store_data_raw; 
wire [1           :0] uncond_jump_inst;
wire [DATA_WIDTH-1:0] cond_branch_target;
wire 								 cond_branch_inst;
wire [DATA_WIDTH-1:0] exu_pc;
wire [DATA_WIDTH-1:0] exu_inst;
assign {regfile_mem_mux, 
				store_inst, 
				load_inst, 
				store_data_raw,
				uncond_jump_inst,
				cond_branch_target,
				cond_branch_inst,
				exu_pc,
				exu_inst
				} = idu_to_exu_bus_r;

wire inst_lw  = load_inst[0];
wire inst_lh  = load_inst[1];
wire inst_lhu = load_inst[2];
wire inst_lb  = load_inst[3];
wire inst_lbu = load_inst[4];

// 由于要8字节对齐，因此有时候需要第2次读
wire need_second_rd;
wire [11					:0] csr_idx;
wire [2						:0] csr_inst;
wire [DATA_WIDTH-1:0] csr_nextpc; 
wire 									csr_nextpc_taken;
assign {csr_idx,
				csr_inst,
				csr_nextpc,
				csr_nextpc_taken
				} = idu_to_exu_csr_bus_r;

reg exu_valid;
always @(posedge clock) begin
	if (reset) 
		exu_valid <= 1'b0;
	else if (exu_allowin)
		exu_valid <= idu_to_exu_valid;
end

always @(posedge clock) begin
	if (reset) begin
		idu_to_exu_alu_bus_r <= 0;
		idu_to_exu_bus_r     <= 0;
		idu_to_exu_csr_bus_r <= 0;
	end 
	else if (exu_allowin && idu_to_exu_valid) begin
		idu_to_exu_alu_bus_r <= idu_to_exu_alu_bus;
		idu_to_exu_bus_r     <= idu_to_exu_bus;
		idu_to_exu_csr_bus_r <= idu_to_exu_csr_bus;
	end
end

wire exu_ready_go;
wire load_ready_go;
wire store_ready_go;

assign load_ready_go  = need_second_rd ? (next_r == SHAKED_R2)
                                : (next_r == SHAKED_R);
assign store_ready_go = (next_w == SHAKED_B);
assign exu_ready_go = |store_inst ? store_ready_go :
											|load_inst  ? load_ready_go :
																		1'b1;

assign exu_allowin = !exu_valid || exu_ready_go;
/*============================ read FSM ========================*/
parameter [3:0] IDLE_R = 4'h0, 
								WAIT_ARREADY = 4'h1, SHAKED_AR = 4'h2,
								WAIT_RVALID = 4'h3, SHAKED_R = 4'h4,
								WAIT_ARREADY2 = 4'h5, SHAKED_AR2 = 4'h6,
								WAIT_RVALID2 = 4'h7, SHAKED_R2 = 4'h8,
								IDLE_R2 = 4'h9;
reg [3:0] state_r, next_r;
always @(posedge clock) begin
	if (reset) 
		state_r <= IDLE_R;
	else 
		state_r <= next_r;
end

wire read_start;
assign read_start = |load_inst && exu_valid;

wire rid_equal;
assign rid_equal = (dsram_arid == dsram_rid);

always @(*) begin
	next_r = IDLE_R;
	case (state_r)
		IDLE_R: 
			if (!read_start) 
				next_r = IDLE_R;
			else if (!dsram_arready)
				next_r = WAIT_ARREADY;
			else 
				next_r = SHAKED_AR;
		WAIT_ARREADY:
			if (dsram_arready)
				next_r = SHAKED_AR;
			else
				next_r = WAIT_ARREADY;
		SHAKED_AR:
			if (dsram_rvalid && rid_equal)
				next_r = SHAKED_R;
			else
				next_r = WAIT_RVALID;
		WAIT_RVALID:
			if (dsram_rvalid && rid_equal)
				next_r = SHAKED_R;
			else 
				next_r = WAIT_RVALID;
		SHAKED_R:
			if (need_second_rd)
				next_r = IDLE_R2;
			else
				next_r = IDLE_R;

// second read
		IDLE_R2:
			if (!dsram_arready)
				next_r = WAIT_ARREADY2;
			else 
				next_r = SHAKED_AR2;
			
		WAIT_ARREADY2:
			if (dsram_arready)
				next_r = SHAKED_AR2;
			else
				next_r = WAIT_ARREADY2;

		SHAKED_AR2:
			if (dsram_rvalid && rid_equal)
				next_r = SHAKED_R2;
			else
				next_r = WAIT_RVALID2;

		WAIT_RVALID2:
			if (dsram_rvalid && rid_equal)
				next_r = SHAKED_R2;
			else 
				next_r = WAIT_RVALID2;

		SHAKED_R2:
				next_r = IDLE_R;

		default: ;
	endcase	
end

reg 			arvalid_r;
reg [3:0] arid_r;
reg [7:0] arlen_r;
reg [1:0] arburst_r;

assign dsram_arvalid = arvalid_r;
assign dsram_arburst = arburst_r;
assign dsram_arlen = arlen_r;
assign dsram_arid = arid_r;

always @(posedge clock) begin
	if (reset) begin
		arvalid_r <= 0;
		arid_r <= 0;
		arlen_r <= 0;
		arburst_r <= 0;	
	end
	else if ((state_r == IDLE_R && next_r == WAIT_ARREADY) 
				|| (state_r == IDLE_R && next_r == SHAKED_AR) 
				|| (state_r == WAIT_ARREADY && next_r == WAIT_ARREADY) 
				|| (next_r == IDLE_R2)
				|| (next_r == WAIT_ARREADY2) )
				/*
				|| (state_r == IDLE_R2 && next_r == SHAKED_AR2) 
				|| (state_r == WAIT_ARREADY2 && next_r == WAIT_ARREADY2)  
				|| (state_r == IDLE_R2 && next_r == WAIT_ARREADY2) ) 
				*/
		begin
		arvalid_r <= 1'b1;
		arid_r <= tik;
		arlen_r <= 8'h0;
		arburst_r <= 0;	
		end
	else begin
		arvalid_r <= 0;
		arid_r <= arid_r;
		arlen_r <= 0;
		arburst_r <= 0;	
		end
end

reg rready_r;
assign dsram_rready = rready_r;
always @(posedge clock) begin
	if (reset) rready_r <= 1'b0;
	else if (next_r == SHAKED_AR 
				|| next_r == WAIT_RVALID
			  || next_r == SHAKED_AR2 
				|| next_r == WAIT_RVALID2)
		rready_r <= 1'b1;
	else
		rready_r <= 1'b0;
end

reg [DATA_WIDTH*2-1:0] first_rdata;
always @(posedge clock) begin
	if (reset) first_rdata <= 0;
	else if (next_r == SHAKED_R) 
		first_rdata <= dsram_rdata; 
end

/* ===================== write FSM =======================*/
parameter [2:0] IDLE_W = 3'b000, WAIT_AWREADY = 3'b001, SHAKED_AW = 3'b010,
								WAIT_WREADY = 3'b011, SHAKED_W = 3'b100, 
								WAIT_BVALID = 3'b101, SHAKED_B = 3'b110;
reg [2:0] state_w, next_w;
always @(posedge clock) begin
	if (reset) 
		state_w <= IDLE_W;
	else 
		state_w <= next_w;
end

wire write_start;
assign write_start = regfile_mem_mux[1] && exu_valid;

wire wid_equal;
assign wid_equal = (dsram_awid == dsram_bid);
always @(wid_equal or state_w or write_start or dsram_awready or dsram_wready or dsram_bvalid) begin
	next_w = IDLE_W;
	case (state_w)
		IDLE_W: 
			if (!write_start) 
				next_w = IDLE_W;
			else if (!dsram_awready)
				next_w = WAIT_AWREADY;
			else 
				next_w = SHAKED_AW;
		WAIT_AWREADY:
			if (!dsram_awready)	
				next_w = WAIT_AWREADY;
			else
				next_w = SHAKED_AW;
		SHAKED_AW:
			if (!dsram_wready)
				next_w = WAIT_WREADY;
			else 
				next_w = SHAKED_W;
		WAIT_WREADY:
			if (!dsram_wready)
				next_w = WAIT_WREADY;
			else 
				next_w = SHAKED_W;
		SHAKED_W:
			if (dsram_bvalid && wid_equal)
				next_w = SHAKED_B;
			else
				next_w = WAIT_BVALID;
		WAIT_BVALID:
			if (dsram_bvalid && wid_equal)
				next_w = SHAKED_B;
			else
				next_w = WAIT_BVALID;
		SHAKED_B:
			if (!write_start)
				next_w = IDLE_W;
			else if (!dsram_awready)
				next_w = WAIT_AWREADY;
			else 
				next_w = SHAKED_AW;
		default: ;
	endcase
end

reg awvalid_r;
//assign dsram_awvalid = (state_w == IDLE_W) ? write_start : awvalid_r;
assign dsram_awvalid = awvalid_r;
reg [3:0] awid_r;
assign dsram_awid = awid_r;
reg [7:0] awlen_r;
assign dsram_awlen = awlen_r;
reg [1:0] awburst_r;
assign dsram_awburst = awburst_r;

always @(posedge clock) begin
	if (reset) begin
		awvalid_r <= 0;
		awid_r <= 0;
		awlen_r <= 8'h0;
		awburst_r <= 0;	
	end
	else if ((state_w == IDLE_W && next_w == WAIT_AWREADY) 
				|| (state_w == IDLE_W && next_w == SHAKED_AW) 
				|| (state_w == WAIT_AWREADY && next_w == WAIT_AWREADY) ) 
		begin
		awvalid_r <= 1'b1;
		awid_r <= tik;
		awlen_r <= 8'h0;
		awburst_r <= 0;	
		end
	else begin
		awvalid_r <= 0;
		awid_r <= awid_r;
		awlen_r <= 8'h0;
		awburst_r <= 0;	
		end
end

reg bready_r;
assign dsram_bready = bready_r;
always @(posedge clock) begin
	if (reset) bready_r <= 1'b0;
	else if (next_w == SHAKED_W || next_w == WAIT_BVALID)
		bready_r <= 1'b1;
	else
		bready_r <= 1'b0;
end


reg wvalid_r;
assign dsram_wvalid = wvalid_r;
reg wlast_r;
assign dsram_wlast = wlast_r;
always @(posedge clock) begin
	if (reset) begin
		wvalid_r <= 1'b0;
		wlast_r <= 1'b0;
	end
	else if (next_w == SHAKED_AW || next_w == WAIT_WREADY) begin
		wvalid_r <= 1'b1;
		wlast_r <= 1'b1;
	end
	else begin
		wvalid_r <= 1'b0;
		wlast_r <= 1'b0;
	end
end

/*=========================================================*/
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
wire [DATA_WIDTH-1:0] exu_nextpc;
wire 									exu_nextpc_taken;
assign exu_nextpc = ({DATA_WIDTH{branch_taken    }} & branch_target) 
									| ({DATA_WIDTH{csr_nextpc_taken}} & csr_nextpc  );
assign exu_nextpc_taken = (branch_taken || csr_nextpc_taken) && exu_valid;
assign exu_to_ifu_bus = {exu_nextpc_taken, exu_nextpc};
assign exu_to_ifu_valid = exu_valid && exu_ready_go;

/* ========== connect with dsram ======================== */
/* =======store instruction ============================== */
assign dsram_awaddr = alu_result; 
// decide awsize;
wire [31:0] uart_addr_min; 
assign uart_addr_min = 32'h1000_0000;
wire [31:0] uart_addr_max;
assign uart_addr_max  = 32'h1000_0fff;
wire write_to_uart;
assign write_to_uart = (dsram_awaddr >= uart_addr_min) &&
								 (dsram_awaddr <= uart_addr_max);
assign dsram_awsize = write_to_uart ? 3'b000 : 3'b010; 
/*
assign dsram_awsize = ({3{write_to_uart | store_inst[2]}} & 3'b000) 
	| ({3{store_inst[1]}} & 3'b001)
	| ({3{store_inst[0]}} & 3'b010);
*/

//assign dsram_wen = regfile_mem_mux[1];
//assign dsram_awvalid = regfile_mem_mux[1];
assign dsram_wdata[31:0] = store_data_raw; 
assign dsram_wdata[63:32] = store_data_raw; 
assign dsram_wstrb[7:4] = 4'b0;
assign dsram_wstrb[3:0] = ( {4{store_inst[0]}} & 4'b1111 )
											| ( {4{store_inst[1]}} & 4'b0011 )
											| ( {4{store_inst[2]}} & 4'b0001 );
/* =======load instruction ============================== */
wire read_from_uart;
assign read_from_uart = (dsram_araddr >= uart_addr_min) &&
								 (dsram_araddr <= uart_addr_max);
assign dsram_arsize = read_from_uart ? 3'b000 : 3'b010; 

wire [31:0] araddr_raw;
assign araddr_raw = alu_result;
wire [31:0] align_low_araddr;
wire [31:0] align_high_araddr;
assign align_low_araddr = {araddr_raw[31:3], 3'b000};
assign align_high_araddr = align_low_araddr + 32'h8;

reg second;
always @(posedge clock) 
	if (reset) second <= 0;
	else if (next_r == IDLE_R2)
		second <= 1'b1;
	else if (state_r == SHAKED_R2)
		second <= 1'b0;

assign dsram_araddr = second ?  
					 align_high_araddr : align_low_araddr;

wire [2:0] sel = araddr_raw[2:0];

assign need_second_rd = (inst_lw && sel == 3'h5) 
							|| (inst_lw && sel == 3'h6) 
							|| (inst_lw && sel == 3'h7) 
							|| ((inst_lh | inst_lhu) && sel == 3'h7);  

wire [DATA_WIDTH-1:0] load_data;


assign load_data = 
	({32{sel == 3'd0 && inst_lw }} & 												 first_rdata[31:0])
| ({32{sel == 3'd0 && inst_lh }} & {{16{first_rdata[15]}}, first_rdata[15:0]})
| ({32{sel == 3'd0 && inst_lhu}} & { 16'd0, 							 first_rdata[15:0]})
| ({32{sel == 3'd0 && inst_lb }} & {{24{first_rdata[7]}},  first_rdata[7:0]})
| ({32{sel == 3'd0 && inst_lbu}} & { 24'd0, 							 first_rdata[7:0]})

|	({32{sel == 3'd1 && inst_lw }} & 												 first_rdata[39:8])
| ({32{sel == 3'd1 && inst_lh }} & {{16{first_rdata[23]}}, first_rdata[23:8]})
| ({32{sel == 3'd1 && inst_lhu}} & { 16'd0, 							 first_rdata[23:8]})
| ({32{sel == 3'd1 && inst_lb }} & {{24{first_rdata[15]}}, first_rdata[15:8]})
| ({32{sel == 3'd1 && inst_lbu}} & { 24'd0, 							 first_rdata[15:8]})

|	({32{sel == 3'd2 && inst_lw }} & 												 first_rdata[47:16])
| ({32{sel == 3'd2 && inst_lh }} & {{16{first_rdata[31]}}, first_rdata[31:16]})
| ({32{sel == 3'd2 && inst_lhu}} & { 16'd0, 							 first_rdata[31:16]})
| ({32{sel == 3'd2 && inst_lb }} & {{24{first_rdata[23]}}, first_rdata[23:16]})
| ({32{sel == 3'd2 && inst_lbu}} & { 24'd0, 							 first_rdata[23:16]})

|	({32{sel == 3'd3 && inst_lw }} & 												 first_rdata[55:24])
| ({32{sel == 3'd3 && inst_lh }} & {{16{first_rdata[39]}}, first_rdata[39:24]})
| ({32{sel == 3'd3 && inst_lhu}} & { 16'd0, 							 first_rdata[39:24]})
| ({32{sel == 3'd3 && inst_lb }} & {{24{first_rdata[31]}}, first_rdata[31:24]})
| ({32{sel == 3'd3 && inst_lbu}} & { 24'd0, 							 first_rdata[31:24]})

|	({32{sel == 3'd4 && inst_lw }} & 												 first_rdata[63:32])
| ({32{sel == 3'd4 && inst_lh }} & {{16{first_rdata[47]}}, first_rdata[47:32]})
| ({32{sel == 3'd4 && inst_lhu}} & { 16'd0, 							 first_rdata[47:32]})
| ({32{sel == 3'd4 && inst_lb }} & {{24{first_rdata[39]}}, first_rdata[39:32]})
| ({32{sel == 3'd4 && inst_lbu}} & { 24'd0, 							 first_rdata[39:32]})

|	({32{sel == 3'd5 && inst_lw }} & {dsram_rdata[7:0]		 , first_rdata[63:40]})
| ({32{sel == 3'd5 && inst_lh }} & {{16{first_rdata[55]}}, first_rdata[55:40]})
| ({32{sel == 3'd5 && inst_lhu}} & { 16'd0, 							 first_rdata[55:40]})
| ({32{sel == 3'd5 && inst_lb }} & {{24{first_rdata[47]}}, first_rdata[47:40]})
| ({32{sel == 3'd5 && inst_lbu}} & { 24'd0, 							 first_rdata[47:40]})

|	({32{sel == 3'd6 && inst_lw }} & {dsram_rdata[15:0]		 , first_rdata[63:48]})
| ({32{sel == 3'd6 && inst_lh }} & {{16{first_rdata[63]}}, first_rdata[63:48]})
| ({32{sel == 3'd6 && inst_lhu}} & { 16'd0, 							 first_rdata[63:48]})
| ({32{sel == 3'd6 && inst_lb }} & {{24{first_rdata[47]}}, first_rdata[55:48]})
| ({32{sel == 3'd6 && inst_lbu}} & { 24'd0, 							 first_rdata[55:48]})

|	({32{sel == 3'd7 && inst_lw }} & {dsram_rdata[23:0]		 , first_rdata[63:56]})
| ({32{sel == 3'd7 && inst_lh }} & {{16{dsram_rdata[7]}}, dsram_rdata[7:0], first_rdata[63:56]})
| ({32{sel == 3'd7 && inst_lhu}} & {16'b0, 							dsram_rdata[7:0], first_rdata[63:56]})
| ({32{sel == 3'd7 && inst_lb }} & {{24{first_rdata[63]}}, first_rdata[63:56]})
| ({32{sel == 3'd7 && inst_lbu}} & { 24'd0, 							 first_rdata[63:56]});


/* ============ to regfile ============================== */
// 若是 jal, jalr, 那么将 rd <- exu_pc + 4
assign regfile_wdata = |uncond_jump_inst ? exu_pc + 4 : 
											 |load_inst     ? load_data :
											 (csr_inst[0] || csr_inst[1]) ? src2 : // csrrw, csrrs
																				alu_result;

assign regfile_waddr = rd;
	// regfile 写使能
wire exu_to_regfile_valid; 
assign exu_to_regfile_valid = exu_valid && exu_ready_go;
assign regfile_wen = regfile_mem_mux[0] && exu_to_regfile_valid;






/* ============= to CSR ================================= */
// csrrw: rd <- csr, csr <- crc1;
// csrrs: rd <- csr, csr <- src1 | csr (这个结果经ALU);
// ecall: mepc <- exu_pc, mcause <- 0xb
assign csr_wdata = csr_inst[0] ? src1 
								: csr_inst[1] ? alu_result // only for csrrs
								: exu_pc; // only for ecall
assign csr_waddr  = csr_inst[2] ? 12'h341 : csr_idx;
									
// csr_wdata2 & csr_waddr2 only for ecall, write to mcause
assign csr_wdata2 = csr_inst[2] ? 32'hb   : 0;
assign csr_waddr2 = csr_inst[2] ? 12'h342 : 0;



assign exu_done[0] = (state_r == SHAKED_R2)
									|| (state_r == SHAKED_R);
assign exu_done[1] = (state_w == SHAKED_B);

/* =============== DPI-C ========================= */
export "DPI-C" task exu_will_go_next_clock_signal;
task exu_will_go_next_clock_signal (output bit o);
	o = exu_to_ifu_valid;
endtask

export "DPI-C" task update_regfile_no;
task update_regfile_no (output [REG_WIDTH-1:0] reg_no);
	reg_no = regfile_wen ? regfile_waddr : 'b0;
endtask

export "DPI-C" task update_regfile_data;
task update_regfile_data (output [DATA_WIDTH-1:0] din);
	din    = regfile_wdata;
endtask

export "DPI-C" task get_inst_from_exu;
task get_inst_from_exu (output [DATA_WIDTH-1:0] din);
	din    = exu_inst;
endtask

// clint
export "DPI-C" task clint_addr_check;
task clint_addr_check (output bit o);
	o = |load_inst && (
			(dsram_araddr >= 32'h0200_0000)
			&& (dsram_araddr <= 32'h0200_ffff)
			);
endtask

// serial 
export "DPI-C" task uart_addr_check;
task uart_addr_check (output bit o);
	o = |store_inst && (
			(dsram_araddr >= 32'h1000_0000)
			&& (dsram_araddr <= 32'h1000_0fff)
			);
endtask

// 初步的 access fault
export "DPI-C" task check_if_access_fault;
task check_if_access_fault (output bit o);
	o    = (dsram_bvalid && dsram_bready) ? 
			(dsram_bresp == 2'b11) : 
			(dsram_rvalid && dsram_rready) ? 
			(dsram_rresp == 2'b11) : 1'b0;
endtask
/*
export "DPI-C" task get_pc_from_exu;
task get_pc_from_exu (output [DATA_WIDTH-1:0] din);
	din    = exu_pc;
endtask
*/

endmodule
