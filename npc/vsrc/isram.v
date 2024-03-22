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
reg [31:0] isram [32'h8000_0000:32'h8000_0031];  
initial $readmemh("/home/chuan/ysyx-workbench/npc/dummy_revised.v", isram);

always @(posedge clk) begin
	if (rst) begin
		inst_o <= 0;
	end else begin
		/*
		inst_o <= {isram[addr+3], isram[addr+2], isram[addr+1], isram[addr]};
		$display("%d", inst_o);
		*/
		inst_o <= isram[addr];
	end
end

endmodule
