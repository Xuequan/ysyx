// 本模块模拟 inst sram, 输入地址，得到指令
module isram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [ADDR_WIDTH-1:0] addr,
	
	output reg [DATA_WIDTH-1:0] inst_o
);
reg [DATA_WIDTH-1:0] isram [32'h8000_0000:32'h8000_0030];  // 暂定1024行
initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy.bin", isram, 32'h8000_0000, 32'h8000_0030);

always @(posedge clk) begin
	if (rst) 
		inst_o <= 0;
	else 
		inst_o <= isram[addr];
end
endmodule
