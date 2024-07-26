// IFU 模块
`include "ysyx_23060208_npc.h"    
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32) (
	input clock,
	input reset,

	/* connect with EXU */
	input [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus,
	input												exu_to_ifu_valid,

	input												exu_allowin,
	/* connect with IDU */
	output [`IFU_TO_IDU_BUS-1:0] ifu_to_idu_bus,  
	output 											 ifu_to_idu_valid,
	input												 idu_valid,

	/* connect with arbiter */
	//input [2						:0] grant,
	output									ifu_done,
	/* connect with isram */
	// 读请求
	input										isram_arready,
	output [DATA_WIDTH-1:0] isram_araddr,
	output									isram_arvalid,
	output [7						:0] isram_arlen,
	output [3						:0] isram_arid,
	output [2						:0] isram_arsize,
	output [1						:0] isram_arburst,

	// 读响应
	output									 isram_rready,
	input										 isram_rvalid,
	input	[1					   :0] isram_rresp,
	input [DATA_WIDTH*2-1:0] isram_rdata,  
	input										 isram_rlast,
	input [3						 :0] isram_rid,

	output									ifu_allowin
);

/* handle data from exu */
wire [DATA_WIDTH-    1:0] exu_nextpc;
wire											exu_nextpc_taken;
//reg [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus_r;
assign {exu_nextpc_taken, exu_nextpc} = exu_to_ifu_bus;

/*
always @(posedge clock) begin
	if (reset) 
		exu_to_ifu_bus_r <= 0;
	else if (ifu_allowin)
		exu_to_ifu_bus_r <= exu_to_ifu_bus;
end
*/
/* ====================  get the nextpc ================*/
wire [DATA_WIDTH-1:0] pc;
wire [DATA_WIDTH-1:0] nextpc;
assign nextpc = (exu_to_ifu_valid && exu_nextpc_taken) ? exu_nextpc : pc + 4;
/* ======================================================== */
// ifu_valid 表示当前 IFU 有有效的数据
reg  ifu_valid;
wire ifu_ready_go;

// 表示 EXU 传过来的数据有效
reg exu_data_valid;
always @(posedge clock) begin
	if (reset)  
		exu_data_valid <= 0;
	else if(ifu_allowin) 	 
	 	exu_data_valid <= exu_to_ifu_valid;
end

/*============================ read FSM ========================*/
parameter [2:0] IDLE_R = 3'b000, WAIT_ARREADY = 3'b001, SHAKED_AR = 3'b010,
                WAIT_RVALID = 3'b011, SHAKED_R = 3'b100;
reg [2:0] state, next;
always @(posedge clock) begin
  if (reset) 
    state <= IDLE_R;
  else 
    state <= next;
end

wire id_equal;
assign id_equal = (isram_rid == isram_arid);

always @(id_equal or state or ifu_allowin or isram_arready or isram_rvalid) begin
  next = IDLE_R;
  case (state)
    IDLE_R: 
      if (!ifu_allowin) 
        next = IDLE_R;
      else if (!isram_arready)
        next = WAIT_ARREADY;
      else 
        next = SHAKED_AR;
    WAIT_ARREADY:
      if (isram_arready)
        next = SHAKED_AR;
      else
        next = WAIT_ARREADY;
    SHAKED_AR:
      if (isram_rvalid && id_equal)
        next = SHAKED_R;
			else 
				next = WAIT_RVALID;
    WAIT_RVALID:
      if (isram_rvalid && id_equal)
        next = SHAKED_R;
      else 
        next = WAIT_RVALID;
    SHAKED_R:
      if (!ifu_allowin)
        next = IDLE_R;
      else if (!isram_arready)
        next = WAIT_ARREADY;
      else 
        next = SHAKED_AR;
    default: ;
  endcase 
end

reg arvalid_r;
assign isram_arvalid = arvalid_r;
reg [7:0] arlen_r;
assign isram_arlen = arlen_r;
reg [3:0] arid_r;
assign isram_arid = arid_r;
assign isram_arsize = 3'b010;
reg [1:0] arburst_r;
assign isram_arburst = arburst_r;

always @(posedge clock) begin
	if (reset) begin
		arvalid_r <= 0;
		arlen_r <= 8'h0;
		arid_r <= 0;
		arburst_r <= 2'b00;
	end
	else if ((state == IDLE_R && next == WAIT_ARREADY) || 
					 (state == IDLE_R && next == SHAKED_AR) ||
					 (state == WAIT_ARREADY && next == WAIT_ARREADY) )
		begin
		arvalid_r <= 1'b1;
		arlen_r <= 8'h0;
		arid_r <= (pc[3:0] == 4'b0) ? 4'h3 : pc[3:0];
		arburst_r <= 2'b00;
		end
	else begin
		arvalid_r <= 1'b0;
		arlen_r <= 8'h0;
		arid_r <= arid_r;
		arburst_r <= 2'b00;
		end
end
reg [DATA_WIDTH-1:0] araddr_r;
assign isram_araddr = araddr_r;
always @(posedge clock) begin
	if (reset) 
		araddr_r <= 0;
	else if ((state == IDLE_R && next == WAIT_ARREADY) ||
					 (state == IDLE_R && next == SHAKED_AR) ||
					 (state == WAIT_ARREADY && next == WAIT_ARREADY) )
		araddr_r <= nextpc;
end

always @(posedge clock) begin
	if (reset) 
		ifu_valid <= 0;
	else if (next == SHAKED_R)
		ifu_valid <= 1'b1;
	else 
		ifu_valid <= 1'b0;
end

/* reveive instruction from isram */
reg [DATA_WIDTH*2-1:0] inst_r;
always @(posedge clock) begin
	if (reset)
		inst_r <= 0;
	else if (next == SHAKED_R) 
		inst_r <= isram_rdata;
end

reg rready_r;
assign isram_rready = rready_r;
always @(posedge clock) begin
	if (reset) rready_r <= 0;
	else if (next == SHAKED_AR || next == WAIT_RVALID)
		rready_r <= 1'b1;
	else
		rready_r <= 1'b0;
end

/*
reg ifu_done_r;
assign ifu_done = ifu_done_r;
always @(posedge clock) begin
	if (reset) ifu_done_r <= 0;
	else if (next == SHAKED_R)
		ifu_done_r <= 1'b1;
	else
		ifu_done_r <= 1'b0;
end
*/
assign ifu_to_idu_bus = {isram_araddr, 
			(isram_araddr[2:0] == 3'b100 ? isram_rdata[63:32] : isram_rdata[31:0])};
//assign ifu_to_idu_bus = {isram_araddr, (isram_araddr[2] ? inst_r[63:32] : inst_r[31:0]) };
assign ifu_ready_go = (next == SHAKED_R);

assign ifu_done = (state == SHAKED_R);

assign ifu_to_idu_valid = ifu_ready_go;
assign ifu_allowin = !idu_valid && exu_allowin && !ifu_valid;

//================= get pc from register PC ==============================
//the program counter PC holds the address of the current instruction.
wire pc_reg_wen;
assign pc_reg_wen = (next == SHAKED_R);
ysyx_23060208_PC #(.DATA_WIDTH(DATA_WIDTH)) PC_i0(
	.clock(clock),
	.reset(reset),
	.wen(pc_reg_wen),    // wen
	.next_pc(araddr_r),  // input
	.pc(pc)              // output
);

/* ==================== DPI-C ====================== */
export "DPI-C" task get_nextPC;
task get_nextPC (output [DATA_WIDTH-1:0] o);
		o = nextpc;
endtask

export "DPI-C" task get_PC;
task get_PC (output [DATA_WIDTH-1:0] o);
	o = pc;
endtask

export "DPI-C" task ifu_ready_go_signal;
task ifu_ready_go_signal (output bit o);
	o = ifu_ready_go;
endtask

export "DPI-C" task check_if_access_fault_ifu;
task check_if_access_fault_ifu (output bit o);
	o = (isram_rready && isram_rvalid) ? 
				(isram_rresp == 2'b11) : 1'b0;
endtask
endmodule
