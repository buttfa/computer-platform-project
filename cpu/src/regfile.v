/*
 * 模块：Regfile寄存器文件
 * 简述：32个64位寄存器组成的寄存器文件，支持读写操作，x0寄存器恒为0
 * 输入：
 *      en          ：使能信号（上升沿触发写操作）
 *      rd          ：目标寄存器索引（写操作）
 *      rs1, rs2    ：源寄存器索引（读操作）
 *      we          ：写使能信号（1有效）
 *      write_data  ：写入目标寄存器的数据
 * 输出：
 *      data1, data2：对应rs1和rs2寄存器的值
 */
module regfile (
    input en, 
    input [4:0] rd,
    input [4:0] rs1,
    input [4:0] rs2,
    output [63:0] data1,
    output [63:0] data2,
    input we,
    input [63:0] write_data
);

    // 寄存器堆定义（x0恒为0）
    reg [63:0] registers [31:0];

    // 写操作由en上升沿触发
    always @(posedge en) begin 
        if (we && rd != 5'b0) begin
            registers[rd] <= write_data;
        end
    end

    // 组合逻辑读操作保持不变
    assign data1 = (rs1 == 5'b0) ? 64'b0 : registers[rs1];
    assign data2 = (rs2 == 5'b0) ? 64'b0 : registers[rs2];

endmodule