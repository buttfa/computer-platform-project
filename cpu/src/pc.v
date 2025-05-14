/*
 * 模块：程序计数器（Program Counter, PC）
 * 简述：生成下一条指令地址，支持顺序执行和跳转控制。
 * 输入：
 *      clk            ：时钟信号
 *      en             ：使能信号（高电平有效）
 *      reset          ：同步复位信号（高电平有效）
 *      tar            ：跳转目标地址（64位）
 *      sign           ：PC更新选择信号（0=PC+4，1=跳转地址）
 * 输出：
 *      pc_addr        ：当前指令地址（64位）
 */
 module pc(
    input               clk,
    input               en,
    input               reset,
    input       [63:0]  tar,
    input               sign,
    output reg  [63:0]  pc_addr
);

//复位地址
localparam reset_add = 64'h00000000;

//组合逻辑计算下一个PC值
wire [63:0] next_pc = sign ? tar : (pc_addr + 64'd4);

//时序逻辑更新PC
always @(posedge clk) begin
    if (en) begin
        if (reset) begin
            pc_addr <= reset_add;
        end else begin
            pc_addr <= next_pc;
        end
    end
end
endmodule