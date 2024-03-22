// 本模块模拟 inst sram, 输入地址，得到指令
module isram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [ADDR_WIDTH-1:0] addr,
	
	output reg [DATA_WIDTH-1:0] inst_o
);
//reg [383:0] isram [32'h8000_0000:32'h8000_f000];  
reg [127:0] isram [32'h8000_0000:32'h8000_f000];  
initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy.bin", isram);

reg [1:0] cnt;
always @(posedge clk) begin
	if (rst) begin
		inst_o <= 0;
		cnt <= 0;
	end else if(cnt == 0) begin
		inst_o <= isram[addr][31:0];
		cnt <= 1;
	end else if(cnt == 2'b01) begin
		inst_o <= isram[addr-32][63:32];
		cnt <= 2'b10;
	end else if(cnt == 2'b10) begin
		inst_o <= isram[addr-64][95:64];
		cnt <= 2'b11;
	end else if(cnt == 2'b11) begin
		inst_o <= isram[addr-96][127:96];
		cnt <= 2'b00;
	end
end
endmodule
