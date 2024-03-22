// 本模块模拟 inst sram, 输入地址，得到指令
module isram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [ADDR_WIDTH-1:0] addr,
	
	output reg [DATA_WIDTH-1:0] inst_o
);
reg [383:0] isram [32'h8000_0000:32'h8000_f000];  
//reg [16*8-1:0] isram [32'h0000_0000:32'h0000_f000];  
initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy.v", isram);

always @(posedge clk) begin
	if (rst) begin
		inst_o <= 0;
	end else begin
		inst_o <= isram[addr][31:0];
	end
end
endmodule
