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

wire inst_sw = store_inst[0];
wire inst_sh = store_inst[1];
wire inst_sb = store_inst[2];

// 由于要8字节对齐，因此有时候需要第2次读/写
wire need_second_rd;
wire need_second_wr;
wire need_second_wr_others;
wire psram_need_second_wr;

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

assign load_ready_go  = need_second_rd ? 
												(next_r == SHAKED_R && second_rd)
                      : (next_r == SHAKED_R);

assign store_ready_go = need_second_wr ? 
								(next_w == SHAKED_B && second_wr)
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
				next_r = WAIT_ARREADY;
			else 
				next_r = SHAKED_AR;

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
assign dsram_rready = rready_r;
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
	else if (dsram_rvalid && dsram_rready && need_second_rd) 
		first_rdata_r <= dsram_rdata; 
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
assign wid_equal = (dsram_awid == dsram_bid);

always @(*) begin
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
			if (need_second_wr) 
				next_w = IDLE_W2;

		IDLE_W2:
			if (!dsram_awready)
				next_w = WAIT_AWREADY;
			else 
				next_w = SHAKED_AW;
			
		default: ;
	endcase
end

reg awvalid_r;
reg [3:0] awid_r;
reg [7:0] awlen_r;
reg [1:0] awburst_r;
assign dsram_awburst = awburst_r;
assign dsram_awvalid = awvalid_r;
assign dsram_awid = awid_r;
assign dsram_awlen = awlen_r;

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
assign dsram_bready = bready_r;
always @(posedge clock) begin
	if (reset) bready_r <= 1'b0;
	else if (next_w == SHAKED_W || next_w == WAIT_BVALID)
		bready_r <= 1'b1;
	else
		bready_r <= 1'b0;
end


reg wvalid_r;
reg wlast_r;
assign dsram_wvalid = wvalid_r;
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
wire [31:0] uart_addr_min; 
wire [31:0] uart_addr_max;
assign uart_addr_min = 32'h1000_0000;
assign uart_addr_max  = 32'h1000_0fff;

wire [31:0] mrom_addr_min; 
wire [31:0] mrom_addr_max;
assign mrom_addr_min = 32'h2000_0000;
assign mrom_addr_max = 32'h2000_0fff;
wire is_mrom_addr;

assign is_mrom_addr = (araddr_raw >= mrom_addr_min) 
									&& (araddr_raw <= mrom_addr_max);

wire [31:0] flash_addr_min; 
wire [31:0] flash_addr_max;
assign flash_addr_min = 32'h3000_0000;
assign flash_addr_max = 32'h3fff_ffff;
wire is_flash_addr;
assign is_flash_addr = (araddr_raw >= flash_addr_min) 
									&& (araddr_raw <= flash_addr_max);

wire [31:0] spi_master_addr_min; 
wire [31:0] spi_master_addr_max;
assign spi_master_addr_min = 32'h1000_1000;
assign spi_master_addr_max  = 32'h1000_1fff;
wire is_spi_master_addr; 
assign is_spi_master_addr = (alu_result >= spi_master_addr_min) 
									&& (alu_result <= spi_master_addr_max);


wire [31:0] psram_addr_min; 
wire [31:0] psram_addr_max;
assign psram_addr_min = 32'h8000_0000;
assign psram_addr_max  = 32'h9fff_ffff;
wire is_psram_addr; 
assign is_psram_addr = (alu_result >= psram_addr_min) 
									&& (alu_result <= psram_addr_max);

wire [31:0] awaddr_raw;
assign awaddr_raw = alu_result;

wire write_to_uart;
assign write_to_uart = (awaddr_raw >= uart_addr_min) &&
								 (awaddr_raw <= uart_addr_max);

assign dsram_awsize = write_to_uart ? 3'b000 
										: is_spi_master_addr ? 3'b010
										: is_psram_addr ? 3'b010
										: 3'b011; 

/* 记录下写数据逻辑。
 * 首先先明确的是：写数据是64bit, 是靠 wstrb 信号控制的；
 * 其次写数据也需要64bit对齐；因此对于某些写地址需要写2次。
 */

wire [31:0] align8_low_awaddr;
wire [31:0] align8_high_awaddr;
assign align8_low_awaddr = {awaddr_raw[31:3], 3'b000};
assign align8_high_awaddr = align8_low_awaddr + 32'h8;

wire [2:0] sel_w; 
assign sel_w = awaddr_raw[2:0];

wire [63:0] store_data;
// 第一次写地址是 0，故先将低位写
// 第二次写地址是 8，故再将高位写
assign store_data = 
	({64{sel_w == 3'd0 && inst_sw }} & {32'b0, store_data_raw} )
| ({64{sel_w == 3'd0 && inst_sh }} & {48'b0, store_data_raw[15:0] })
| ({64{sel_w == 3'd0 && inst_sb }} & {32'b0, 24'b0, store_data_raw[7:0]})

|	({64{sel_w == 3'd1 && inst_sw }} & {24'b0, store_data_raw, 8'b0})
| ({64{sel_w == 3'd1 && inst_sh }} & {32'b0, 8'b0,  store_data_raw[15:0], 8'b0})
| ({64{sel_w == 3'd1 && inst_sb }} & {32'b0, 16'b0, store_data_raw[7:0], 8'b0})

|	({64{sel_w == 3'd2 && inst_sw }} & {16'b0, store_data_raw, 16'b0} )
| ({64{sel_w == 3'd2 && inst_sh }} & {32'b0, store_data_raw[15:0], 16'b0} )
| ({64{sel_w == 3'd2 && inst_sb }} & {32'b0, 8'b0, store_data_raw[7:0], 16'b0})

|	({64{sel_w == 3'd3 && inst_sw }} & {8'b0,  store_data_raw, 24'b0})
| ({64{sel_w == 3'd3 && inst_sh }} & {24'b0, store_data_raw[15:0], 24'b0})
| ({64{sel_w == 3'd3 && inst_sb }} & {32'b0, store_data_raw[7:0], 24'b0})

|	({64{sel_w == 3'd4 && inst_sw }} & {store_data_raw, 32'b0})
| ({64{sel_w == 3'd4 && inst_sh }} & {16'b0, store_data_raw[15:0], 32'b0})
| ({64{sel_w == 3'd4 && inst_sb }} & {24'b0, store_data_raw[7:0], 32'b0})

|	({64{sel_w == 3'd5 && inst_sw }} & {store_data_raw[23:0], 32'b0	, store_data_raw[31:24]})
| ({64{sel_w == 3'd5 && inst_sh }} & {8'b0, store_data_raw[15:0], 40'b0})
| ({64{sel_w == 3'd5 && inst_sb }} & {16'b0, store_data_raw[7:0], 40'b0})

|	({64{sel_w == 3'd6 && inst_sw }} & {store_data_raw[15:0], 32'b0, store_data_raw[31:16]})
| ({64{sel_w == 3'd6 && inst_sh }} & {store_data_raw[15:0], 48'b0})
| ({64{sel_w == 3'd6 && inst_sb }} & {8'b0, store_data_raw[7:0], 48'b0})

|	({64{sel_w == 3'd7 && inst_sw }} & {store_data_raw[7:0], 32'b0, store_data_raw[31:8]})
| ({64{sel_w == 3'd7 && inst_sh }} & {store_data_raw[7:0], 48'b0, store_data_raw[15:8]})
| ({64{sel_w == 3'd7 && inst_sb }} & {store_data_raw[7:0], 56'b0});

// 第一次写的 wstrb
wire [7:0] wstrb;
// 第二次写的 wstrb
wire [7:0] wstrb2;
assign wstrb = 
	({8{sel_w == 3'd0 && inst_sw }} & 8'b0000_1111)
| ({8{sel_w == 3'd1 && inst_sw }} & 8'b0001_1110)
| ({8{sel_w == 3'd2 && inst_sw }} & 8'b0011_1100)
| ({8{sel_w == 3'd3 && inst_sw }} & 8'b0111_1000)
| ({8{sel_w == 3'd4 && inst_sw }} & 8'b1111_0000)
| ({8{sel_w == 3'd5 && inst_sw }} & 8'b1110_0000)   
| ({8{sel_w == 3'd6 && inst_sw }} & 8'b1100_0000)   
| ({8{sel_w == 3'd7 && inst_sw }} & 8'b1000_0000)   

| ({8{sel_w == 3'd0 && inst_sh }} & 8'b0000_0011)   
| ({8{sel_w == 3'd1 && inst_sh }} & 8'b0000_0110)   
| ({8{sel_w == 3'd2 && inst_sh }} & 8'b0000_1100)   
| ({8{sel_w == 3'd3 && inst_sh }} & 8'b0001_1000)   
| ({8{sel_w == 3'd4 && inst_sh }} & 8'b0011_0000)   
| ({8{sel_w == 3'd5 && inst_sh }} & 8'b0110_0000)   
| ({8{sel_w == 3'd6 && inst_sh }} & 8'b1100_0000)   

| ({8{sel_w == 3'd0 && inst_sb }} & 8'b0000_0001)   
| ({8{sel_w == 3'd1 && inst_sb }} & 8'b0000_0010)   
| ({8{sel_w == 3'd2 && inst_sb }} & 8'b0000_0100)   
| ({8{sel_w == 3'd3 && inst_sb }} & 8'b0000_1000)   
| ({8{sel_w == 3'd4 && inst_sb }} & 8'b0001_0000)   
| ({8{sel_w == 3'd5 && inst_sb }} & 8'b0010_0000)   
| ({8{sel_w == 3'd6 && inst_sb }} & 8'b0100_0000)   
| ({8{sel_w == 3'd7 && inst_sb }} & 8'b1000_0000) ;   
// 第二次写
assign wstrb2 = 
	({8{sel_w == 3'd5 && inst_sw }} & 8'b0000_0001)   
| ({8{sel_w == 3'd6 && inst_sw }} & 8'b0000_0011)   
| ({8{sel_w == 3'd7 && inst_sw }} & 8'b0000_0111)   

| ({8{sel_w == 3'd7 && inst_sh }} & 8'b1000_0001);   


assign need_second_wr_others = (inst_sw && sel == 3'h5) 
							|| (inst_sw && sel == 3'h6) 
							|| (inst_sw && sel == 3'h7) 
							|| (inst_sh && sel == 3'h7);  

assign need_second_wr = is_psram_addr ? psram_need_second_wr : need_second_wr_others; 

reg second_wr;
always @(posedge clock) 
	if (reset) second_wr <= 0;
	else if (next_w == IDLE_W2)
		second_wr <= 1'b1;
	else if (state_w == IDLE_W)
		second_wr <= 1'b0;

//assign sel_w = awaddr_raw[2:0];
wire [7:0] spi_master_wstrb;
assign spi_master_wstrb = ({8{inst_sw}} & 8'b1111_1111) 
				| ({8{inst_sh}} & 8'b0011_0011)
				| ({8{inst_sb}} & 8'b0001_0001);

assign dsram_wstrb  = is_spi_master_addr ? spi_master_wstrb
						: (is_psram_addr && !second_wr) ? {2{psram_first_wstrb}}
						: (is_psram_addr &&  second_wr) ? {2{psram_second_wstrb}}
						: second_wr ? wstrb2 : wstrb;

assign dsram_awaddr = write_to_uart ? awaddr_raw 
										: is_spi_master_addr ? awaddr_raw
										: is_psram_addr ? psram_awaddr
										: second_wr ? align8_high_awaddr 
										: align8_low_awaddr;

assign dsram_wdata = is_spi_master_addr ? {2{store_data_raw}} 
									 : is_psram_addr ? {2{store_data_raw}}
									 : store_data;

/* ======== psram store signal start========================= */
wire [3:0] psram_first_wstrb;
assign psram_first_wstrb = 
	({4{sel_w == 3'd0 && inst_sw }} & 4'b1111)
| ({4{sel_w == 3'd1 && inst_sw }} & 4'b1110)
| ({4{sel_w == 3'd2 && inst_sw }} & 4'b1100)
| ({4{sel_w == 3'd3 && inst_sw }} & 4'b1000)
| ({4{sel_w == 3'd4 && inst_sw }} & 4'b1111)
| ({4{sel_w == 3'd5 && inst_sw }} & 4'b1110)   
| ({4{sel_w == 3'd6 && inst_sw }} & 4'b1100)   
| ({4{sel_w == 3'd7 && inst_sw }} & 4'b1000)   

| ({4{sel_w == 3'd0 && inst_sh }} & 4'b0011)   
| ({4{sel_w == 3'd1 && inst_sh }} & 4'b0110)   
| ({4{sel_w == 3'd2 && inst_sh }} & 4'b1100)   
| ({4{sel_w == 3'd3 && inst_sh }} & 4'b1000)   
| ({4{sel_w == 3'd4 && inst_sh }} & 4'b0011)   
| ({4{sel_w == 3'd5 && inst_sh }} & 4'b0110)   
| ({4{sel_w == 3'd6 && inst_sh }} & 4'b1100)   
| ({4{sel_w == 3'd7 && inst_sh }} & 4'b1000)   

| ({4{sel_w == 3'd0 && inst_sb }} & 4'b0001)   
| ({4{sel_w == 3'd1 && inst_sb }} & 4'b0010)   
| ({4{sel_w == 3'd2 && inst_sb }} & 4'b0100)   
| ({4{sel_w == 3'd3 && inst_sb }} & 4'b1000)   
| ({4{sel_w == 3'd4 && inst_sb }} & 4'b0001)   
| ({4{sel_w == 3'd5 && inst_sb }} & 4'b0010)   
| ({4{sel_w == 3'd6 && inst_sb }} & 4'b0100)   
| ({4{sel_w == 3'd7 && inst_sb }} & 4'b1000);   
wire [3:0] psram_second_wstrb;
assign psram_second_wstrb = 
	({4{sel_w == 3'd1 && inst_sw }} & 4'b0001)
| ({4{sel_w == 3'd2 && inst_sw }} & 4'b0011)
| ({4{sel_w == 3'd3 && inst_sw }} & 4'b0111)
| ({4{sel_w == 3'd5 && inst_sw }} & 4'b0001)   
| ({4{sel_w == 3'd6 && inst_sw }} & 4'b0011)   
| ({4{sel_w == 3'd7 && inst_sw }} & 4'b0111)   

| ({4{sel_w == 3'd3 && inst_sh }} & 4'b0001)   
| ({4{sel_w == 3'd7 && inst_sh }} & 4'b0001);

assign psram_need_second_wr = (inst_sw && sel_w != 3'h0 && sel_w != 3'h4) 
	|| (inst_sh && sel_w == 3'h3)
	|| (inst_sh && sel_w == 3'h7); 

wire [31:0] psram_awaddr = 
		({32{(sel_w <= 3'b011 && !second_wr)}} & {awaddr_raw[31:3], 3'b000})
	| ({32{(sel_w <= 3'b011 &&  second_wr)}} & {awaddr_raw[31:3], 3'b100})
	| ({32{(sel_w >= 3'b100 && !second_wr)}} & {awaddr_raw[31:3], 3'b100})  // 1st
	| ({32{(sel_w >= 3'b100 && 	second_wr)}} & {awaddr_raw[31:4], 4'b1000});//2st


/* ======== psram store signal end========================= */
/* =======load instruction ============================== */
wire [31:0] araddr_raw;
assign araddr_raw = alu_result;

wire read_from_uart;
assign read_from_uart = (araddr_raw >= uart_addr_min) &&
								 (araddr_raw <= uart_addr_max);
assign dsram_arsize = read_from_uart ? 3'b000 : 3'b010; 

wire [31:0] align8_low_araddr;
wire [31:0] align8_high_araddr;
wire [31:0] align4_araddr;
assign align8_low_araddr = {araddr_raw[31:3], 3'b000};
assign align8_high_araddr = align8_low_araddr + 32'h8;

assign align4_araddr = {araddr_raw[31:2], 2'b00};

wire [31:0] read_from_flash_araddr;
assign read_from_flash_araddr = align4_araddr;


reg second_rd;
always @(posedge clock) 
	if (reset) second_rd <= 0;
	else if (next_r == IDLE_R2)
		second_rd <= 1'b1;
	else if (state_r == IDLE_R)
		second_rd <= 1'b0;

/* 这里记录下我的逻辑：
 * 1. 需明白 MROM 是有 DPI-C 读取内存的值；且每次读取 32bit；返回的是64bit，
 * rdata; rdata[31:0] 是有效的；rdata[63:32] 是无效的（实际上与[31:0]相等；
 * 2. 同时需明白，传递给 MROM 的地址会被4bytes对齐（也就是令[1:0] == 2'b00），
 * 然后该对齐后的地址通过 DPI-C 传回到 pmem-read() 读数据；
 * 3. 而 SRAM 是不同的；SoC 里实现了它：mem_1024x64；
 * 4. 因此对于 SRAM 的读，每次都得8bytes 地址对齐！！每次读回来的数据是64bit，
 * 都是有效的。
 *
 * 基于以上事实，我将 EXU 的读分为了两部分：
 * 	一部分是向 MROM 读，此时只需 4byte 对齐地址；
 * 		这部分是有问题的，我偷懒了，因为向 MROM 读数据（不是读指令）是只有
 * 		init_data() 这部分的，而它值用到了 lbu, 因此我只保证了 lbu 读到的数据
 * 		是正确的！！
 * 		这部分相关的信号是：sel2, align4_araddr, load_data2;
 * 	另一部分是向 SRAM 读，此时需 8byte 对齐，而且有时候需要二次读；
 * 		这部分相关的信号是: sel, align8_low_araddr, align8_high_araddr,
 * 		load_data; 
 */
//assign align4_araddr = {araddr_raw[31:2], 2'b00};
assign dsram_araddr = read_from_uart ? align4_araddr 
										: is_spi_master_addr ? align4_araddr
										: is_psram_addr ? align4_araddr
										: is_flash_addr ? read_from_flash_araddr 
										: is_mrom_addr ? align4_araddr 
										: second_rd ?  align8_high_araddr 
										: align8_low_araddr;

wire [2:0] sel = araddr_raw[2:0];
wire [1:0] sel2 = araddr_raw[1:0];

assign need_second_rd = (inst_lw && sel == 3'h5) 
							|| (inst_lw && sel == 3'h6) 
							|| (inst_lw && sel == 3'h7) 
							|| ((inst_lh | inst_lhu) && sel == 3'h7);  

wire [DATA_WIDTH-1:0] load_data;
// 仅针对从 mrom 中读取的数据
wire [DATA_WIDTH-1:0] load_data2;

assign load_data2 = 
	({32{sel2 == 2'd0 && inst_lw }} & 												 dsram_rdata[31:0])
| ({32{sel2 == 2'd0 && inst_lh }} & {{16{dsram_rdata[15]}}, dsram_rdata[15:0]})
| ({32{sel2 == 2'd0 && inst_lhu}} & { 16'd0, 							 dsram_rdata[15:0]})
| ({32{sel2 == 2'd0 && inst_lb }} & {{24{dsram_rdata[7]}},  dsram_rdata[7:0]})
| ({32{sel2 == 2'd0 && inst_lbu}} & { 24'd0, 							 dsram_rdata[7:0]})

|	({32{sel2 == 2'd1 && inst_lw }} & 												 dsram_rdata[39:8])
| ({32{sel2 == 2'd1 && inst_lh }} & {{16{dsram_rdata[23]}}, dsram_rdata[23:8]})
| ({32{sel2 == 2'd1 && inst_lhu}} & { 16'd0, 							 dsram_rdata[23:8]})
| ({32{sel2 == 2'd1 && inst_lb }} & {{24{dsram_rdata[15]}}, dsram_rdata[15:8]})
| ({32{sel2 == 2'd1 && inst_lbu}} & { 24'd0, 							 dsram_rdata[15:8]})

|	({32{sel2 == 2'd2 && inst_lw }} & 												 dsram_rdata[47:16])
| ({32{sel2 == 2'd2 && inst_lh }} & {{16{dsram_rdata[31]}}, dsram_rdata[31:16]})
| ({32{sel2 == 2'd2 && inst_lhu}} & { 16'd0, 							 dsram_rdata[31:16]})
| ({32{sel2 == 2'd2 && inst_lb }} & {{24{dsram_rdata[23]}}, dsram_rdata[23:16]})
| ({32{sel2 == 2'd2 && inst_lbu}} & { 24'd0, 							 dsram_rdata[23:16]})

|	({32{sel2 == 2'd3 && inst_lw }} & 												 dsram_rdata[55:24])
| ({32{sel2 == 2'd3 && inst_lh }} & {{16{dsram_rdata[39]}}, dsram_rdata[39:24]})
| ({32{sel2 == 2'd3 && inst_lhu}} & { 16'd0, 							 dsram_rdata[39:24]})
| ({32{sel2 == 2'd3 && inst_lb }} & {{24{dsram_rdata[31]}}, dsram_rdata[31:24]})
| ({32{sel2 == 2'd3 && inst_lbu}} & { 24'd0, 							 dsram_rdata[31:24]});


wire [63:0] first_rdata;
assign first_rdata = need_second_rd ? first_rdata_r : dsram_rdata;

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


/*
// 仅针对对 psram 读取的数据
wire [31:0] load_data3;
//wire [2:0] sel = araddr_raw[2:0];
//wire [1:0] sel2 = araddr_raw[1:0];
assign load_data3 = 
	({32{sel2 == 2'd0 && inst_lw }} & 												 dsram_rdata[31:0])
| ({32{sel2 == 2'd0 && inst_lh }} & {{16{dsram_rdata[15]}}, dsram_rdata[15:0]})
| ({32{sel2 == 2'd0 && inst_lhu}} & { 16'd0, 							 dsram_rdata[15:0]})
| ({32{sel2 == 2'd0 && inst_lb }} & {{24{dsram_rdata[7]}},  dsram_rdata[7:0]})
| ({32{sel2 == 2'd0 && inst_lbu}} & { 24'd0, 							 dsram_rdata[7:0]})

|	({32{sel2 == 2'd1 && inst_lw }} & 												 dsram_rdata[39:8])
| ({32{sel2 == 2'd1 && inst_lh }} & {{16{dsram_rdata[23]}}, dsram_rdata[23:8]})
| ({32{sel2 == 2'd1 && inst_lhu}} & { 16'd0, 							 dsram_rdata[23:8]})
| ({32{sel2 == 2'd1 && inst_lb }} & {{24{dsram_rdata[15]}}, dsram_rdata[15:8]})
| ({32{sel2 == 2'd1 && inst_lbu}} & { 24'd0, 							 dsram_rdata[15:8]})

|	({32{sel2 == 2'd2 && inst_lw }} & 												 dsram_rdata[47:16])
| ({32{sel2 == 2'd2 && inst_lh }} & {{16{dsram_rdata[31]}}, dsram_rdata[31:16]})
| ({32{sel2 == 2'd2 && inst_lhu}} & { 16'd0, 							 dsram_rdata[31:16]})
| ({32{sel2 == 2'd2 && inst_lb }} & {{24{dsram_rdata[23]}}, dsram_rdata[23:16]})
| ({32{sel2 == 2'd2 && inst_lbu}} & { 24'd0, 							 dsram_rdata[23:16]})

|	({32{sel2 == 2'd3 && inst_lw }} & 												 dsram_rdata[55:24])
| ({32{sel2 == 2'd3 && inst_lh }} & {{16{dsram_rdata[39]}}, dsram_rdata[39:24]})
| ({32{sel2 == 2'd3 && inst_lhu}} & { 16'd0, 							 dsram_rdata[39:24]})
| ({32{sel2 == 2'd3 && inst_lb }} & {{24{dsram_rdata[31]}}, dsram_rdata[31:24]})
| ({32{sel2 == 2'd3 && inst_lbu}} & { 24'd0, 							 dsram_rdata[31:24]});
*/
/* ============ to regfile ============================== */
// 若是 jal, jalr, 那么将 rd <- exu_pc + 4
assign regfile_wdata = |uncond_jump_inst ? exu_pc + 4 : 
											 |load_inst && is_mrom_addr    ? load_data2 :
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



assign exu_done[0] = (state_r == SHAKED_R);
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
export "DPI-C" task uart_write_check;
task uart_write_check (output bit o);
	o = |store_inst && write_to_uart; 
endtask
export "DPI-C" task uart_read_check;
task uart_read_check (output bit o);
	o = |load_inst && read_from_uart; 
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
	o    = (dsram_bvalid && dsram_bready) ? 
			(dsram_bresp == 2'b11) : 
			(dsram_rvalid && dsram_rready) ? 
			(dsram_rresp == 2'b11) : 1'b0;
endtask
endmodule
