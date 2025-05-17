/*
 * 模块：ramfile
 * 简述：实现64位总线同步读写RAM，支持大端字节序存储
 * 输入：
 *      clk        ：时钟信号（上升沿触发写入）
 *      reset      ：同步复位信号（高电平有效）
 *      mem_write  ：写使能信号（1=写入，0=读取）
 *      addr       ：64位地址总线（实际使用低24位）
 *      data_in    ：64位写入数据
 * 输出：
 *      data_out   ：64位读取数据
 */
module ramfile (
    input               clk,
    input               reset,
    input               mem_write, 
    input       [63:0]  addr,
    input       [63:0]  data_in,
    output reg  [63:0]  data_out
);

// RAM存储阵列（64位数据宽度，28位地址空间）
reg [63:0] ram [0:(1<<28)-1];

// 大端字节序转换函数 
function [63:0] store;
    input [63:0] data;
    begin
        store = {  // 拆分为8个字节，地址递增方向存储
            data[63:56],
            data[55:48],
            data[47:40],
            data[39:32],
            data[31:24],
            data[23:16],
            data[15:8],
            data[7:0]
        };
    end
endfunction

// 大端字节序加载函数，转换为符合计算机内部处理方式的顺序
function [63:0] load;
    input [63:0] stored_data;
    begin
        load = {  // 从存储体字节序重组数据
            stored_data[63:56],
            stored_data[55:48],
            stored_data[47:40],
            stored_data[39:32],
            stored_data[31:24],
            stored_data[23:16],
            stored_data[15:8],
            stored_data[7:0]
        };
    end
endfunction

// 同步写入逻辑（时钟上升沿触发）
always @(posedge clk) begin
    if (reset) begin
        // 复位时可选的初始化操作（下为示例，全部清零）
        for (integer i=0; i<(1<<28); i=i+1) ram[i] <= 64'b0;
    end 
    else if (mem_write) begin
        // 大端模式写入：地址截取28位，数据转换存储格式
        ram[addr[27:0]] <= store(data_in); 
    end
end

// 组合逻辑读取（实时响应地址变化）
always @(*) begin
    // 大端模式读取：地址截取24位，数据转换输出格式
    data_out = load(ram[addr[27:0]]);
end

endmodule