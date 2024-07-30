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
	input										axi_awready,
	output [DATA_WIDTH-1:0] axi_awaddr,
	output									axi_awvalid,
	
	/* 新增 */
	output [3						:0] axi_awid,
	output [7						:0] axi_awlen,
	output [2						:0] axi_awsize,
	output [1						:0] axi_awburst,

	// 写数据通道
	output 										axi_wvalid,
	output [DATA_WIDTH*2-1:0] axi_wdata, 
	output [7							:0] axi_wstrb,
	output 										axi_wlast,
	input											axi_wready,
	// 写响应通道
	input										 axi_bvalid,
	input  [1						 :0] axi_bresp,
	input	 [3						 :0] axi_bid,
	output									 axi_bready,

	// 读请求通道
	input										axi_arready,
	output									axi_arvalid, 
	output [DATA_WIDTH-1:0] axi_araddr,
	output [3						:0] axi_arid,
	output [7						:0] axi_arlen,
	output [2						:0] axi_arsize,
	output [1						:0] axi_arburst,
	// 读响应通道
	output									 axi_rready,
	input										 axi_rvalid,
	input [DATA_WIDTH*2-1:0] axi_rdata,  
	input [1:							0] axi_rresp,
	input										 axi_rlast,
	input [3:							0] axi_rid,

	
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

wire inst_sw = store_inst[0];
wire inst_sh = store_inst[1];
wire inst_sb = store_inst[2];

// 有时候需要第2次读/写
wire need_second_r;
wire need_second_w;
// make the second write
reg second_w;
// mark the second read
reg second_r;

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

assign load_ready_go  = need_second_r ? 
												(next_r == SHAKED_R && second_r)
                      : (next_r == SHAKED_R);

assign store_ready_go = need_second_w ? 
								(next_w == SHAKED_B && second_w)
							: (next_w == SHAKED_B);

assign exu_ready_go = |store_inst ? store_ready_go :
											|load_inst  ? load_ready_go :
																		1'b1;

assign exu_allowin = !exu_valid || exu_ready_go;
/*============================ read FSM ========================*/

parameter [2:0] IDLE_R = 3'h0, 
								WAIT_ARREADY = 3'h1, SHAKED_AR = 3'h2,
								WAIT_RVALID = 3'h3, SHAKED_R = 3'h4,
								IDLE_R2 = 3'h5;

reg [2:0] state_r, next_r;
always @(posedge clock) begin
	if (reset) 
		state_r <= IDLE_R;
	else 
		state_r <= next_r;
end

wire read_start;
assign read_start = |load_inst && exu_valid;

wire rid_equal;
assign rid_equal = (axi_arid == axi_rid);

always @(*) begin
	next_r = IDLE_R;
	case (state_r)
		IDLE_R: 
			if (!read_start) 
				next_r = IDLE_R;
			else if (!axi_arready)
				next_r = WAIT_ARREADY;
			else 
				next_r = SHAKED_AR;
		WAIT_ARREADY:
			if (axi_arready)
				next_r = SHAKED_AR;
			else
				next_r = WAIT_ARREADY;
		SHAKED_AR:
			if (axi_rvalid && rid_equal)
				next_r = SHAKED_R;
			else
				next_r = WAIT_RVALID;
		WAIT_RVALID:
			if (axi_rvalid && rid_equal)
				next_r = SHAKED_R;
			else 
				next_r = WAIT_RVALID;
		SHAKED_R:
			if (need_second_r && !second_r)
				next_r = IDLE_R2;
			else
				next_r = IDLE_R;

// second read
		IDLE_R2:
			if (!axi_arready)
				next_r = WAIT_ARREADY;
			else 
				next_r = SHAKED_AR;

		default: ;
	endcase	
end

always @(posedge clock) 
	if (reset) second_r <= 0;
	else if (next_r == IDLE_R2)
		second_r <= 1'b1;
	else if (state_r == IDLE_R)
		second_r <= 1'b0;


reg 			arvalid_r;
reg [3:0] arid_r;
reg [7:0] arlen_r;
reg [1:0] arburst_r;

assign axi_arvalid = arvalid_r;
assign axi_arburst = arburst_r;
assign axi_arlen = arlen_r;
assign axi_arid = arid_r;

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
				|| (state_r == IDLE_R2 && next_r == SHAKED_AR) 
				|| (state_r == IDLE_R2 && next_r == WAIT_ARREADY)) 
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
assign axi_rready = rready_r;
always @(posedge clock) begin
	if (reset) rready_r <= 1'b0;
	else if (next_r == SHAKED_AR 
				|| next_r == WAIT_RVALID)
		rready_r <= 1'b1;
	else
		rready_r <= 1'b0;
end

reg [DATA_WIDTH*2-1:0] first_rdata_r;
always @(posedge clock) begin
	if (reset) first_rdata_r <= 0;
	else if (axi_rvalid && axi_rready && need_second_r) 
		first_rdata_r <= axi_rdata; 
	else if (state_r == IDLE_R)
		first_rdata_r <= 0;
end

/* ===================== write FSM =======================*/
parameter [2:0] IDLE_W = 3'h0, WAIT_AWREADY = 3'h1, SHAKED_AW = 3'h2,
								WAIT_WREADY = 3'h3, SHAKED_W = 3'h4, 
								WAIT_BVALID = 3'h5, SHAKED_B = 3'h6,
								IDLE_W2 = 3'h7;
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
assign wid_equal = (axi_awid == axi_bid);

always @(*) begin
	next_w = IDLE_W;
	case (state_w)
		IDLE_W: 
			if (!write_start) 
				next_w = IDLE_W;
			else if (!axi_awready)
				next_w = WAIT_AWREADY;
			else 
				next_w = SHAKED_AW;
		WAIT_AWREADY:
			if (!axi_awready)	
				next_w = WAIT_AWREADY;
			else
				next_w = SHAKED_AW;
		SHAKED_AW:
			if (!axi_wready)
				next_w = WAIT_WREADY;
			else 
				next_w = SHAKED_W;
		WAIT_WREADY:
			if (!axi_wready)
				next_w = WAIT_WREADY;
			else 
				next_w = SHAKED_W;
		SHAKED_W:
			if (axi_bvalid && wid_equal)
				next_w = SHAKED_B;
			else
				next_w = WAIT_BVALID;
		WAIT_BVALID:
			if (axi_bvalid && wid_equal)
				next_w = SHAKED_B;
			else
				next_w = WAIT_BVALID;
		SHAKED_B:
			if (need_second_w && !second_w) 
				next_w = IDLE_W2;

		IDLE_W2:
			if (!axi_awready)
				next_w = WAIT_AWREADY;
			else 
				next_w = SHAKED_AW;
			
		default: ;
	endcase
end

always @(posedge clock) 
	if (reset) second_w <= 0;
	else if (next_w == IDLE_W2)
		second_w <= 1'b1;
	else if (state_w == IDLE_W)
		second_w <= 1'b0;

reg awvalid_r;
reg [3:0] awid_r;
reg [7:0] awlen_r;
reg [1:0] awburst_r;
assign axi_awburst = awburst_r;
assign axi_awvalid = awvalid_r;
assign axi_awid = awid_r;
assign axi_awlen = awlen_r;

always @(posedge clock) begin
	if (reset) begin
		awvalid_r <= 0;
		awid_r <= 0;
		awlen_r <= 8'h0;
		awburst_r <= 0;	
	end
	else if ((state_w == IDLE_W && next_w == WAIT_AWREADY) 
				|| (state_w == IDLE_W && next_w == SHAKED_AW) 
				|| (state_w == WAIT_AWREADY && next_w == WAIT_AWREADY) 
				|| (state_w == IDLE_W2 && next_w == SHAKED_AW)
				|| (state_w == IDLE_W2 && next_w == WAIT_AWREADY) )
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
assign axi_bready = bready_r;
always @(posedge clock) begin
	if (reset) bready_r <= 1'b0;
	else if (next_w == SHAKED_W || next_w == WAIT_BVALID)
		bready_r <= 1'b1;
	else
		bready_r <= 1'b0;
end


reg wvalid_r;
reg wlast_r;
assign axi_wvalid = wvalid_r;
assign axi_wlast = wlast_r;

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

/* =======clint ============================== */
wire [31:0] addr_raw;
assign addr_raw = alu_result;



wire [31:0] clint_addr_min; 
wire [31:0] clint_addr_max;
assign clint_addr_min = 32'h0200_0000;
assign clint_addr_max  = 32'h0200_ffff;
wire is_clint_addr;
assign is_clint_addr = (addr_raw >= clint_addr_min) &&
								 (addr_raw <= clint_addr_max);
/* =======uart ============================== */
wire [31:0] uart_addr_min; 
wire [31:0] uart_addr_max;
assign uart_addr_min = 32'h1000_0000;
assign uart_addr_max  = 32'h1000_0fff;
wire is_uart_addr;
assign is_uart_addr = (addr_raw >= uart_addr_min) &&
								 (addr_raw <= uart_addr_max);

/* ======= sram ============================== */
wire [31:0] sram_addr_min; 
wire [31:0] sram_addr_max;
assign sram_addr_min = 32'h0f00_0000;
assign sram_addr_max  = 32'h0f00_2000;
wire is_sram_addr;
assign is_sram_addr = (addr_raw >= sram_addr_min) &&
								 (addr_raw <= sram_addr_max);
/* ======= mrom ============================== */
wire [31:0] mrom_addr_min; 
wire [31:0] mrom_addr_max;
assign mrom_addr_min = 32'h2000_0000;
assign mrom_addr_max = 32'h2000_0fff;
wire is_mrom_addr;

assign is_mrom_addr = (addr_raw >= mrom_addr_min) 
									&& (addr_raw <= mrom_addr_max);

/* ======= flash ============================== */
wire [31:0] flash_addr_min; 
wire [31:0] flash_addr_max;
assign flash_addr_min = 32'h3000_0000;
assign flash_addr_max = 32'h3fff_ffff;
wire is_flash_addr;
assign is_flash_addr = (addr_raw >= flash_addr_min) 
									&& (addr_raw <= flash_addr_max);


/* ======= spi master ============================== */
wire [31:0] spi_master_addr_min; 
wire [31:0] spi_master_addr_max;
assign spi_master_addr_min = 32'h1000_1000;
assign spi_master_addr_max  = 32'h1000_1fff;
wire is_spi_master_addr; 
assign is_spi_master_addr = (alu_result >= spi_master_addr_min) 
									&& (alu_result <= spi_master_addr_max);


/* ======= psram ============================== */
wire [31:0] psram_addr_min; 
wire [31:0] psram_addr_max;
assign psram_addr_min = 32'h8000_0000;
assign psram_addr_max = 32'h9fff_ffff;
wire is_psram_addr; 
assign is_psram_addr = (alu_result >= psram_addr_min) 
									&& (alu_result <= psram_addr_max);

/* ======= sdram ============================== */
wire [31:0] sdram_addr_min; 
wire [31:0] sdram_addr_max;
assign sdram_addr_min = 32'ha000_0000;
assign sdram_addr_max = 32'hbfff_ffff;
wire is_sdram_addr; 
assign is_sdram_addr = (alu_result >= sdram_addr_min) 
									&& (alu_result <= psram_addr_max);

/* =========================================================================
/* ======= some checks ========================================================
 * =========================================================================
 */
/* assertion: invalid address of load or store */
always @(*) begin
	if ( exu_valid && 
			!(is_clint_addr || is_uart_addr || is_sram_addr 
				|| is_mrom_addr || is_flash_addr
				|| is_spi_master_addr || is_psram_addr	
				|| is_sdram_addr) ) begin
		if (|load_inst) begin
			$fwrite(32'h8000_0002, "Assertion, EXU module, load addr '%h' is not valid\n", addr_raw);
			$fatal;
		end
		if (|store_inst) begin
			$fwrite(32'h8000_0002, "Assertion, EXU module, write addr '%h' is not valid\n", addr_raw);
			$fatal;
		end
	end
end

/* =========================================================================
/* ======= AXI commom ========================================================
 * =========================================================================
 */
wire [2:0] addr_sel;
assign addr_sel = addr_raw[2:0];

wire sel0 = addr_sel == 3'd0;
wire sel1 = addr_sel == 3'd1;
wire sel2 = addr_sel == 3'd2;
wire sel3 = addr_sel == 3'd3;
wire sel4 = addr_sel == 3'd4;
wire sel5 = addr_sel == 3'd5;
wire sel6 = addr_sel == 3'd6;
wire sel7 = addr_sel == 3'd7;

assign need_second_r = (inst_lw && addr_sel != 3'h0 && addr_sel != 3'h4) 
						 || ( (inst_lh | inst_lhu) && (addr_sel == 3'h3 || addr_sel == 3'h7) );

assign need_second_w = (inst_sw && addr_sel != 3'h0 && addr_sel != 3'h4) 
						 || (inst_sh && (addr_sel == 3'h3 || addr_sel == 3'h7));

/* second read or write */
wire second;
assign second = (|load_inst && second_r) 
							| (|store_inst && second_w);

//------------------------------------
// get addr
// ------------------------------------
wire rw_word = inst_sw | inst_lw; 
wire rw_half = inst_sh | inst_lh | inst_lhu;
wire rw_byte = inst_sb | inst_lb | inst_lbu;

wire [31:0] align4_addr;
assign align4_addr = {addr_raw[31:2], 2'd0};

reg [31:0] rw_byte_addr;
always @(addr_raw) begin
	rw_byte_addr = align4_addr;
	case (addr_raw[3:2])
		2'b00: rw_byte_addr = {addr_raw[31:4], 4'b0000};
		2'b01: rw_byte_addr = {addr_raw[31:4], 4'b0100};
		2'b10: rw_byte_addr = {addr_raw[31:4], 4'b1000};
		2'b11:
			begin 
			if (is_sram_addr)
				rw_byte_addr = {addr_raw[31:4], 4'b1000};
			else
				rw_byte_addr = {addr_raw[31:4], 4'b1100};
			end
		default:;
	endcase
end

/*
wire [31:0] first_addr;
assign first_addr = (rw_word | rw_half) ? align4_addr
									: rw_byte_addr;
*/

reg [31:0] second_addr;
always @(addr_raw)  begin
	second_addr = align4_addr;
	case (addr_raw[3:0])
		4'b0001, 4'b0010, 4'b0011: second_addr = {addr_raw[31:3], 3'b100};
		4'b0101, 4'b0110, 4'b0111: second_addr = {addr_raw[31:4], 4'b1000};
		4'b1001, 4'b1010, 4'b1011: second_addr = {addr_raw[31:4], 4'b1100};
		4'b1101, 4'b1110, 4'b1111: second_addr = {addr_raw[31:5], 5'b10000};
		default:;
	endcase
end

//------------------------------------
// get strb
// ------------------------------------

wire [7:0] first_strb;
assign first_strb = 
	({8{sel0 && rw_word }} & 8'b0000_1111)
| ({8{sel1 && rw_word }} & 8'b0000_1110)
| ({8{sel2 && rw_word }} & 8'b0000_1100)
| ({8{sel3 && rw_word }} & 8'b0000_1000)
| ({8{sel4 && rw_word }} & 8'b1111_0000)
| ({8{sel5 && rw_word }} & 8'b1110_0000)   
| ({8{sel6 && rw_word }} & 8'b1100_0000)   
| ({8{sel7 && rw_word }} & 8'b1000_0000)   

| ({8{sel0 && rw_half }} & 8'b0000_0011)   
| ({8{sel1 && rw_half }} & 8'b0000_0110)   
| ({8{sel2 && rw_half }} & 8'b0000_1100)   
| ({8{sel3 && rw_half }} & 8'b0000_1000)   
| ({8{sel4 && rw_half }} & 8'b0011_0000)   
| ({8{sel5 && rw_half }} & 8'b0110_0000)   
| ({8{sel6 && rw_half }} & 8'b1100_0000)   
| ({8{sel7 && rw_half }} & 8'b1000_0000)   

| ({8{sel0 && rw_byte }} & 8'b0000_0001)   
| ({8{sel1 && rw_byte }} & 8'b0000_0010)   
| ({8{sel2 && rw_byte }} & 8'b0000_0100)   
| ({8{sel3 && rw_byte }} & 8'b0000_1000)   
| ({8{sel4 && rw_byte }} & 8'b0001_0000)   
| ({8{sel5 && rw_byte }} & 8'b0010_0000)   
| ({8{sel6 && rw_byte }} & 8'b0100_0000)   
| ({8{sel7 && rw_byte }} & 8'b1000_0000);   

wire [7:0] second_strb_t;
assign second_strb_t = 
	({8{sel1 && rw_word }} & 8'b0001_0000)
| ({8{sel2 && rw_word }} & 8'b0011_0000)
| ({8{sel3 && rw_word }} & 8'b0111_0000)
| ({8{sel5 && rw_word }} & 8'b0000_0001)   
| ({8{sel6 && rw_word }} & 8'b0000_0011)   
| ({8{sel7 && rw_word }} & 8'b0000_0111)   

| ({8{sel3 && rw_half }} & 8'b0001_0000)   
| ({8{sel7 && rw_half }} & 8'b0000_0001);

/* 为方便后面的 load data */
wire [7:0] second_strb = (need_second_w || need_second_r) ? second_strb_t : 8'b0;
//------------------------------------
// get wdata 
// ------------------------------------
reg [63:0] cal_wdata;

	always @(store_data_raw or addr_sel) begin
		cal_wdata = {32'b0, store_data_raw};
		case (addr_sel)
			3'b000: cal_wdata = {32'b0, store_data_raw};
			3'b001: cal_wdata = {24'b0, store_data_raw, 8'b0};
			3'b010: cal_wdata = {16'b0, store_data_raw, 16'b0};
			3'b011: cal_wdata = {8'b0, store_data_raw, 24'b0};
			3'b100: cal_wdata = {store_data_raw, 32'b0};
			3'b101: cal_wdata = {store_data_raw[23:0], 32'b0, store_data_raw[31:24]};
			3'b110: cal_wdata = {store_data_raw[15:0], 32'b0, store_data_raw[31:16]};
			3'b111: cal_wdata = {store_data_raw[7:0], 32'b0, store_data_raw[31:8]};
			default:;
		endcase
	end
			
//------------------------------------
// get wsize
// ------------------------------------
/*
wire [2:0] first_size;
wire [2:0] second_size;

assign first_size = rw_word ? (3'd4 - {1'b0, addr_raw[1:0]}) 
									: rw_half ? (addr[1:0]  == 2'd3 ? 3'd1 : 3'd2)
									: 3'd1;

assign second_size = rw_word ? ({1'b0, addr[1:0]})
									: rw_half ? 3'd1 
									: 3'd0;
*/

/* =========================================================================
/* ======= store  ==========================================================
 * =========================================================================
 */
/*
wire rw_word = inst_sw | inst_lw; 
wire rw_half = inst_sh | inst_lh | inst_lhu;
wire rw_byte = inst_sb | inst_lb | inst_lbu;
*/
/* axi_awsize */
assign axi_awsize = is_uart_addr ? 3'b000 : 3'b010; 

/* axi_awaddr */
assign axi_awaddr  = second_w ? second_addr : addr_raw;

/* axi_wstrb */
assign axi_wstrb  = second_w ? second_strb : first_strb;

/* axi_wdata */
assign axi_wdata = cal_wdata;

/* =========================================================================
/* ======= load = ==========================================================
 * =========================================================================
 */
/* axi_araddr */
assign axi_araddr  = second_r ? second_addr : addr_raw;

/* axi_arsize */
assign axi_arsize = is_uart_addr ? 3'b000 : 3'b010; 

/* load_data 是最后写入到寄存器中的数据 */
wire [DATA_WIDTH-1:0] load_data;

wire [63:0] first_rdata;
assign first_rdata = need_second_r ? first_rdata_r : axi_rdata;

wire [7:0] byte0 = ({8{first_strb[0]}} & first_rdata[7:0]) 
								| ({8{first_strb[4]}} & first_rdata[39:32])
								| ({8{second_strb[0]}} & axi_rdata[7:0])
								| ({8{second_strb[4]}} & axi_rdata[39:32]);
   
wire [7:0] byte1 = ({8{first_strb[1]}} & first_rdata[15:8]) 
								| ({8{first_strb[5]}} & first_rdata[47:40])
								| ({8{second_strb[1]}} & axi_rdata[15:8])
								| ({8{second_strb[5]}} & axi_rdata[47:40]);


wire [7:0] byte2 = ({8{first_strb[2]}} & first_rdata[23:16]) 
								| ({8{first_strb[6]}} & first_rdata[55:48])
								| ({8{second_strb[2]}} & axi_rdata[23:16])
								| ({8{second_strb[6]}} & axi_rdata[55:48]);


wire [7:0] byte3 = ({8{first_strb[3]}} & first_rdata[31:24]) 
								| ({8{first_strb[7]}} & first_rdata[63:56])
								| ({8{second_strb[3]}} & axi_rdata[31:24])
								| ({8{second_strb[7]}} & axi_rdata[63:56]);


/* prepare data for load  */
wire [31:0] lw_data = 
	({32{sel0 | sel4}} & {byte3, byte2, byte1, byte0})
|	({32{sel1 | sel5}} & {byte0, byte3, byte2, byte1})
|	({32{sel2 | sel6}} & {byte1, byte0, byte3, byte2})
|	({32{sel3 | sel7}} & {byte2, byte1, byte0, byte3});

wire [31:0] lh_data = 
	({32{sel0 | sel4}} & { {16{byte1[7]}}, byte1, byte0})
|	({32{sel1 | sel5}} & { {16{byte2[7]}}, byte2, byte1})
|	({32{sel2 | sel6}} & { {16{byte3[7]}}, byte3, byte2})
|	({32{sel3 | sel7}} & { {16{byte0[7]}}, byte0, byte3});

wire [31:0] lhu_data = 
	({32{sel0 | sel4}} & { 16'b0, byte1, byte0})
|	({32{sel1 | sel5}} & { 16'b0, byte2, byte1})
|	({32{sel2 | sel6}} & { 16'b0, byte3, byte2})
|	({32{sel3 | sel7}} & { 16'b0, byte0, byte3});

wire [31:0] lb_data = 
	({32{sel0 | sel4}} & { {24{byte0[7]}}, byte0})
|	({32{sel1 | sel5}} & { {24{byte1[7]}}, byte1})
|	({32{sel2 | sel6}} & { {24{byte2[7]}}, byte2})
|	({32{sel3 | sel7}} & { {24{byte3[7]}}, byte3});

wire [31:0] lbu_data = 
	({32{sel0 | sel4}} & { 24'b0, byte0})
|	({32{sel1 | sel5}} & { 24'b0, byte1})
|	({32{sel2 | sel6}} & { 24'b0, byte2})
|	({32{sel3 | sel7}} & { 24'b0, byte3});

assign load_data = ({32{inst_lw }} & lw_data)
								 | ({32{inst_lh }} & lh_data)
								 | ({32{inst_lhu}} & lhu_data)
								 | ({32{inst_lb }} & lb_data)
								 | ({32{inst_lbu}} & lbu_data);



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


/* =============connect with internect ============== */
// exu read done
assign exu_done[0] = !need_second_r ? (state_r == SHAKED_R)
				: (state_r == SHAKED_R && second_r);

assign exu_done[1] = !need_second_w ? (state_w == SHAKED_B) 
				: (state_w == SHAKED_B && second_w);


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
	o = |load_inst && is_clint_addr;
endtask

// serial 
export "DPI-C" task uart_write_check;
task uart_write_check (output bit o);
	o = |store_inst && is_uart_addr; 
endtask
export "DPI-C" task uart_read_check;
task uart_read_check (output bit o);
	o = |load_inst && is_uart_addr; 
endtask

// spi master
export "DPI-C" task spi_master_write_check;
task spi_master_write_check (output bit o);
	o = |store_inst && is_spi_master_addr; 
endtask
export "DPI-C" task spi_master_read_check;
task spi_master_read_check (output bit o);
	o = |load_inst && is_spi_master_addr; 
endtask
// 初步的 access fault
export "DPI-C" task check_if_access_fault;
task check_if_access_fault (output bit o);
	o    = (axi_bvalid && axi_bready) ? 
			(axi_bresp == 2'b11) : 
			(axi_rvalid && axi_rready) ? 
			(axi_rresp == 2'b11) : 1'b0;
endtask
endmodule
