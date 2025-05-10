/*
模块：RAM模块
简述：提供16Gx8bit的RAM模块，支持读写操作，使用大端序存储。
输入：
    clk  ：时钟信号
    wr   ：写读信号，当wr=1时为写操作，否则为读操作
    addr ：地址总线，保持64bit输入，但实际使用低34位（16G地址空间）
    data ：数据总线，64位
输出：
    data ：数据总线，64位
*/
module ram (
    input wire clk,
    input wire wr,
    input wire [63:0] addr, 
    
    inout wire [63:0] data
);

/* 存储阵列为16G x 8bit（2^34 x 8bit），即16GB的存储空间 */
reg [7:0] mem [0: (1 << 34) - 1];

/* 用于存储实际访问的地址（转换为integer类型避免警告） */
integer mem_addr;
always @(*) begin
    /* verilator lint_off WIDTH */
    // 显式转换为无符号整数
    mem_addr = addr[33:0];
    /* verilator lint_off WIDTH */
end

// 读操作：组合8个连续字节为64bit（使用整数索引）
wire [63:0] read_data;
assign read_data = {mem[mem_addr+0], mem[mem_addr+1], mem[mem_addr+2], mem[mem_addr+3],
                    mem[mem_addr+4], mem[mem_addr+5], mem[mem_addr+6], mem[mem_addr+7]};
assign data = (!wr) ? read_data : 64'bz;

// 写操作：拆分64bit数据到8个存储单元（使用整数索引）
always @(posedge clk) 
begin
    if (wr) 
    begin
        mem[mem_addr+0] <= data[63:56];
        mem[mem_addr+1] <= data[55:48];
        mem[mem_addr+2] <= data[47:40];
        mem[mem_addr+3] <= data[39:32];
        mem[mem_addr+4] <= data[31:24];
        mem[mem_addr+5] <= data[23:16];
        mem[mem_addr+6] <= data[15:8];
        mem[mem_addr+7] <= data[7:0];
    end
end

endmodule