// 本模块模拟 inst sram, 输入地址，得到指令
module isram
	#(DATA_WIDTH = 32, ADDR_WIDTH = 32) (
	input clk,
	input rst,
	input [ADDR_WIDTH-1:0] addr,
	output reg [DATA_WIDTH-1:0] inst_o
);

/*
reg [7:0] isram [32'h8000_0000:32'h8000_0300];  
initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy.v", isram);
*/
//initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy_revised.v", isram, 32'h80000000, 32'h8000002C);

// 可行
reg [7:0] isram [32'h800_0000:32'h800_0010];  
initial $readmemh("/home/chuan/ysyx-workbench/npc/d2.txt", isram);
/*
// 不可行，不知道为什么，加一位地址就不行了
reg [7:0] isram [32'h8000_0000:32'h8000_0010];  
initial $readmemh("/home/chuan/ysyx-workbench/npc/d2.txt", isram);
*/

always @(posedge clk) begin
	if (rst) begin
		inst_o <= 32'h0;
	end else begin
		inst_o <= {isram[addr+3], isram[addr+2], isram[addr+1], isram[addr]};
		$display("here %x", isram[32'h8000_0002]);
		$display("addr %x", inst_o);
		//inst_o <= isram[addr];
	end
end

endmodule
