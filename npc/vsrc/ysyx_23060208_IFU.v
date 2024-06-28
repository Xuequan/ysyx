// IFU 模块
`include "ysyx_23060208_npc.h"    
module ysyx_23060208_IFU 
	#(DATA_WIDTH = 32) (
	input clk,
	input rst,

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
	output [DATA_WIDTH-1:0] isram_araddr,
	output									isram_arvalid,
	input										isram_arready,
	// 读响应
	input [DATA_WIDTH-1:0]  isram_rdata,  
	input										isram_rvalid,
	input	[1					 :0]  isram_rresp,
	output									isram_rready,

	output									ifu_allowin
);

/* handle data from exu */
wire [DATA_WIDTH-    1:0] exu_nextpc;
wire											exu_nextpc_taken;
//reg [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus_r;
assign {exu_nextpc_taken, exu_nextpc} = exu_to_ifu_bus;

/*
always @(posedge clk) begin
	if (rst) 
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
always @(posedge clk) begin
	if (rst)  
		exu_data_valid <= 0;
	else if(ifu_allowin) 	 
	 	exu_data_valid <= exu_to_ifu_valid;
end

/*============================ read FSM ========================*/
parameter [2:0] IDLE_R = 3'b000, WAIT_ARREADY = 3'b001, SHAKED_AR = 3'b010,
                WAIT_RVALID = 3'b011, SHAKED_R = 3'b100;
reg [2:0] state, next;
always @(posedge clk) begin
  if (rst) 
    state <= IDLE_R;
  else 
    state <= next;
end

always @(state or ifu_allowin or isram_arready or isram_rvalid) begin
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
      if (!isram_rvalid)
        next = WAIT_RVALID;
      else 
        next = SHAKED_R;
    WAIT_RVALID:
      if (isram_rvalid)
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
always @(posedge clk) begin
	if (rst) arvalid_r <= 0;
	else if ((state == IDLE_R && next == WAIT_ARREADY) || 
					 (state == IDLE_R && next == SHAKED_AR) ||
					 (state == WAIT_ARREADY && next == WAIT_ARREADY) )
		arvalid_r <= 1'b1;
	else
		arvalid_r <= 1'b0;
end
reg [DATA_WIDTH-1:0] araddr_r;
assign isram_araddr = araddr_r;
always @(posedge clk) begin
	if (rst) 
		araddr_r <= 0;
	else if ((state == IDLE_R && next == WAIT_ARREADY) ||
					 (state == IDLE_R && next == SHAKED_AR) ||
					 (state == WAIT_ARREADY && next == WAIT_ARREADY) )
		araddr_r <= nextpc;
end

always @(posedge clk) begin
	if (rst) 
		ifu_valid <= 0;
	else if (next == SHAKED_R)
		ifu_valid <= 1'b1;
	else 
		ifu_valid <= 1'b0;
end

/* reveive instruction from isram */
reg [DATA_WIDTH-1:0] inst_r;
always @(posedge clk) begin
	if (rst)
		inst_r <= 0;
	else if (next == SHAKED_R) 
		inst_r <= isram_rdata;
end

reg rready_r;
assign isram_rready = rready_r;
always @(posedge clk) begin
	if (rst) rready_r <= 0;
	else if (next == SHAKED_AR || next == WAIT_RVALID)
		rready_r <= 1'b1;
	else
		rready_r <= 1'b0;
end

/*
reg ifu_done_r;
assign ifu_done = ifu_done_r;
always @(posedge clk) begin
	if (rst) ifu_done_r <= 0;
	else if (next == SHAKED_R)
		ifu_done_r <= 1'b1;
	else
		ifu_done_r <= 1'b0;
end
*/


assign ifu_to_idu_bus = {isram_araddr, isram_rdata};
assign ifu_ready_go = (next == SHAKED_R);

assign ifu_done = (state == SHAKED_R);

assign ifu_to_idu_valid = ifu_ready_go;
assign ifu_allowin = !idu_valid && exu_allowin && !ifu_valid;

//================= get pc from register PC ==============================
//the program counter PC holds the address of the current instruction.
wire pc_reg_wen;
assign pc_reg_wen = (next == SHAKED_R);
ysyx_23060208_PC #(.DATA_WIDTH(DATA_WIDTH)) PC_i0(
	.clk(clk),
	.rst(rst),
	.wen(pc_reg_wen),  // wen
	.next_pc(nextpc),  // input
	.pc(pc)        // output
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
endmodule
