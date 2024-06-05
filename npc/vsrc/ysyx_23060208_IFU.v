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
	//input												 idu_allowin,
	input												 idu_valid,

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
reg [`EXU_TO_IFU_BUS-1:0] exu_to_ifu_bus_r;
assign {exu_nextpc_taken, exu_nextpc} = exu_to_ifu_bus_r;

always @(posedge clk) begin
	if (rst) 
		exu_to_ifu_bus_r <= 0;
	else if (ifu_allowin)
		exu_to_ifu_bus_r <= exu_to_ifu_bus;
end

/* ====================  get the nextpc ================*/
wire [DATA_WIDTH-1:0] ifu_pc;
wire [DATA_WIDTH-1:0] nextpc;
reg [DATA_WIDTH-1:0] nextpc_r;
assign nextpc = nextpc_r;
always @(posedge clk) begin
	if(rst) 
		nextpc_r <= DATA_WIDTH'('h8000_0000);
	else if (state == IDLE_R && next == WAIT_ARREADY)   
		nextpc_r <= (exu_nextpc_taken && exu_data_valid) ? exu_nextpc : ifu_pc + 4;
end
/*
always @(posedge clk) begin
	if(rst) nextpc_r <= DATA_WIDTH'('h8000_0000);
	else if(exu_to_ifu_valid)
		nextpc_r <= (exu_nextpc_taken && exu_data_valid) ? exu_nextpc :
													ifu_pc + 4;
end
*/
/* ======================================================== */
// ifu_valid 表示当前 IFU 有有效的数据
reg ifu_valid;
wire ifu_ready_go;

// 表示 EXU 传过来的数据有效
reg exu_data_valid;
always @(posedge clk) begin
	if (rst)  
		exu_data_valid <= 0;
	else if(ifu_allowin) 	 
	 	exu_data_valid <= exu_to_ifu_valid;
end

assign ifu_to_idu_valid = ifu_valid && ifu_ready_go;
//assign ifu_allowin = !ifu_valid || (ifu_ready_go && idu_allowin);
assign ifu_allowin = !idu_valid && exu_allowin && !ifu_valid;



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

always @(state or ifu_allowin or isram_arvalid or isram_arready or isram_rvalid or isram_rready) begin
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
      if (!isram_rready)
        next = SHAKED_AR;
      else if (!isram_rvalid)
        next = WAIT_RVALID;
      else 
        next = SHAKED_R;
    WAIT_RVALID:
      if (isram_rvalid)
        next = SHAKED_R;
      else 
        next = WAIT_RVALID;
    SHAKED_R:
      if (!isram_arvalid)
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
					 (state == WAIT_ARREADY && next == WAIT_ARREADY) )
		arvalid_r <= 1'b1;
	else
		arvalid_r <= 1'b0;
end

reg ifu_ready_go_r;
assign ifu_ready_go = ifu_ready_go_r;
always @(posedge clk) begin
	if (rst) ifu_ready_go_r <= 0;
	else if (next == SHAKED_R)
		ifu_ready_go_r <= 1'b1;
	else 
		ifu_ready_go_r <= 0;
end

always @(posedge clk) begin
	if (rst) ifu_valid <= 0;
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



assign isram_araddr = nextpc;
assign ifu_to_idu_bus = {ifu_pc, isram_rdata};

//================= get pc from register PC ==============================
wire pc_reg_wen;
assign pc_reg_wen = (next == SHAKED_R);
ysyx_23060208_PC #(.DATA_WIDTH(DATA_WIDTH)) PC_i0(
	.clk(clk),
	.rst(rst),
	.wen(pc_reg_wen),
	.next_pc(nextpc),
	.pc(ifu_pc)
);




/* ==================== DPI-C ====================== */
export "DPI-C" task get_nextPC;
task get_nextPC (output [DATA_WIDTH-1:0] o);
	o = nextpc;
endtask
endmodule
