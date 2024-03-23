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
reg [7:0] isram [0:9];  
//initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy_revised.v", isram, 32'h80000000, 32'h8000002C);

initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy_revised.v", isram, 0, 7);

always @(posedge clk) begin
	if (rst) begin
		inst_o <= 32'h0;
	end else begin
		inst_o <= {isram[addr+3], isram[addr+2], isram[addr+1], isram[addr]};
		$display("%d", inst_o);
		//inst_o <= isram[addr];
	end
end

endmodule
