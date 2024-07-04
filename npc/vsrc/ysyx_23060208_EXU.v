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

assign load_ready_go  = (next_r == SHAKED_R);
assign store_ready_go = (next_w == SHAKED_B);
assign exu_ready_go = |store_inst ? store_ready_go :
											|load_inst  ? load_ready_go :
																		1'b1;

assign exu_allowin = !exu_valid || exu_ready_go;
/*============================ read FSM ========================*/
parameter [2:0] IDLE_R = 3'b000, WAIT_ARREADY = 3'b001, SHAKED_AR = 3'b010,
								WAIT_RVALID = 3'b011, SHAKED_R = 3'b100;
reg [2:0] state_r, next_r;
always @(posedge clock) begin
	if (reset) 
		state_r <= IDLE_R;
	else 
		state_r <= next_r;
end

wire read_start;
//assign read_start = |load_inst && (exu_valid || idu_to_exu_valid);
assign read_start = |load_inst && exu_valid;

always @(state_r or read_start or dsram_arready or dsram_rvalid) begin
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
			if (!dsram_rvalid)
				next_r = WAIT_RVALID;
			else 
				next_r = SHAKED_R;
		WAIT_RVALID:
			if (dsram_rvalid)
				next_r = SHAKED_R;
			else 
				next_r = WAIT_RVALID;
		SHAKED_R:
			if (!read_start)
				next_r = IDLE_R;
			else if (!dsram_arready)
				next_r = WAIT_ARREADY;
			else 
				next_r = SHAKED_AR;
		default: ;
	endcase	
end

//assign dsram_arvalid = |load_inst && exu_valid;
reg arvalid_r;
assign dsram_arvalid = (state_r == IDLE_R) ? read_start : arvalid_r;
reg [3:0] arid_r;
assign dsram_arid = arid_r;
reg [7:0] arlen_r;
assign dsram_arlen = arlen_r;
reg [1:0] arburst_r;
assign dsram_arburst = arburst_r;

always @(posedge clock) begin
	if (reset) arvalid_r <= 0;
	else if ((state_r == IDLE_R && next_r == WAIT_ARREADY) 
				|| (state_r == IDLE_R && next_r == SHAKED_AR) 
				|| (state_r == WAIT_ARREADY && next_r == WAIT_ARREADY) ) 
		begin
		arvalid_r <= 1'b1;
		arid_r <= 0;
		arlen_r <= 8'h3;
		arburst_r <= 0;	
		end
	else begin
		arvalid_r <= 0;
		arid_r <= 0;
		arlen_r <= 0;
		arburst_r <= 0;	
		end
end
reg rready_r;
assign dsram_rready = rready_r;
always @(posedge clock) begin
	if (reset) rready_r <= 1'b0;
	else if (next_r == SHAKED_AR || next_r == WAIT_RVALID)
		rready_r <= 1'b1;
	else
		rready_r <= 1'b0;
end
/*
reg [DATA_WIDTH-1:0] rdata_r;
always @(posedge clock) begin
	if (reset) rdata_r <= 0;
	else if (next_r == SHAKED_R) 
		rdata_r <= dsram_rdata; 
end
always @(posedge clock) begin
	if (reset) load_ready_go <= 0;
	else if (next_r == SHAKED_R) 
		load_ready_go <= 1'b1;
	else 
		load_ready_go <= 0;
end
*/

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

always @(state_w or write_start or dsram_awready or dsram_wready or dsram_bvalid) begin
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
			if (!dsram_bvalid)
				next_w = WAIT_BVALID;
			else
				next_w = SHAKED_B;
		WAIT_BVALID:
			if (!dsram_bvalid)
				next_w = WAIT_BVALID;
			else
				next_w = SHAKED_B;
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
assign dsram_awvalid = (state_w == IDLE_W) ? write_start : awvalid_r;
reg [3:0] awid_r;
assign dsram_awid = awid_r;
reg [7:0] awlen_r;
assign dsram_awlen = awlen_r;
reg [1:0] awburst_r;
assign dsram_awburst = awburst_r;

always @(posedge clock) begin
	if (reset) awvalid_r <= 0;
	else if ((state_w == IDLE_W && next_w == WAIT_AWREADY) 
				|| (state_w == IDLE_W && next_w == SHAKED_AW) 
				|| (state_w == WAIT_AWREADY && next_w == WAIT_AWREADY) ) 
		begin
		awvalid_r <= 1'b1;
		awid_r <= 0;
		awlen_r <= 8'h3;
		awburst_r <= 0;	
		end
	else begin
		awvalid_r <= 0;
		awid_r <= 0;
		awlen_r <= 8'h3;
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
reg [4:0] wstrb_r;
assign dsram_wstrb[7:3] = wstrb_r;
reg wlast_r;
assign dsram_wlast = wlast_r;
always @(posedge clock) begin
	if (reset) wvalid_r <= 1'b0;
	else if (next_w == SHAKED_AW || next_w == WAIT_WREADY) begin
		wvalid_r <= 1'b1;
		wstrb_r <= 5'b0;
		wlast_r <= 1'b1;
	end
	else begin
		wvalid_r <= 1'b0;
		wstrb_r <= 5'b0;
		wlast_r <= 1'b0;
	end
end

/*
always @(posedge clock) begin
	if (reset) store_ready_go <= 0;
	else if (next_w == SHAKED_B) 
		store_ready_go <= 1'b1;
	else 
		store_ready_go <= 0;
end
*/
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
assign uart_addr_min = 32'h2000_0000;
wire [31:0] uart_addr_max;
assign uart_addr_max  = 32'h2000_0fff;
wire write_to_uart;
assign write_to_uart = (dsram_awaddr >= uart_addr_min) &&
								 (dsram_awaddr <= uart_addr_max);
assign dsram_awsize = write_to_uart ? 3'b000 : 3'b010; 
//assign dsram_wen = regfile_mem_mux[1];
//assign dsram_awvalid = regfile_mem_mux[1];
assign dsram_wdata[31:0] = store_data_raw; 
assign dsram_wstrb[2:0] = ( {3{store_inst[0]}} & 3'b100 )
											| ( {3{store_inst[1]}} & 3'b010 )
											| ( {3{store_inst[2]}} & 3'b001 );
/* =======load instruction ============================== */
assign dsram_araddr = alu_result;
wire read_from_uart;
assign read_from_uart = (dsram_araddr >= uart_addr_min) &&
								 (dsram_araddr <= uart_addr_max);
assign dsram_arsize = read_from_uart ? 3'b000 : 3'b010; 

wire [DATA_WIDTH-1:0] load_data;
assign load_data = ({DATA_WIDTH{load_inst[0]}} & dsram_rdata[31:0])
| ({DATA_WIDTH{load_inst[1]}} & {{16{dsram_rdata[15]}}, dsram_rdata[15:0]})
| ({DATA_WIDTH{load_inst[2]}} & { 16'b0, dsram_rdata[15:0]})
| ({DATA_WIDTH{load_inst[3]}} & {{24{dsram_rdata[7]}}, dsram_rdata[7:0]})
| ({DATA_WIDTH{load_inst[4]}} & { 24'b0, dsram_rdata[7:0]});

/* ============ to regfile ============================== */
// 若是 jal, jalr, 那么将 rd <- exu_pc + 4
assign regfile_wdata = |uncond_jump_inst ? exu_pc + 4 : 
											 |load_inst     ? load_data :
											 (csr_inst[0] || csr_inst[1]) ? src2 : // csrrw, csrrs
																				alu_result;

assign regfile_waddr = rd;
	// regfile 写使能
assign regfile_wen = regfile_mem_mux[0] && exu_valid;






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



assign exu_done[0] = (state_r == SHAKED_R);
assign exu_done[1] = (state_w == SHAKED_B);

/* =============== DPI-C ========================= */
export "DPI-C" task exu_ready_go_signal;
task exu_ready_go_signal (output bit o);
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

// rtc
export "DPI-C" task rtc_addr_check;
task rtc_addr_check (output bit o);
	o = |load_inst && (
			(dsram_araddr == 32'ha000_0048)
			|| (dsram_araddr == 32'ha000_0048 + 4)
			);
endtask

// serial 
export "DPI-C" task uart_addr_check;
task uart_addr_check (output bit o);
	o = |store_inst && (
			(dsram_araddr == 32'ha000_03f8)
			);
endtask
/*
export "DPI-C" task get_pc_from_exu;
task get_pc_from_exu (output [DATA_WIDTH-1:0] din);
	din    = exu_pc;
endtask
*/

endmodule
