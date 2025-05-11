/*
 * 模块：ALU模块
 * 简述：提供64位运算单元，支持加减乘除、位移、逻辑运算。
 * 输入：
 *      clk  ：时钟信号
 *      en   ：使能信号
 *      opcode ：操作码
 *      operand1 ：操作数A
 *      operand2 ：操作数B
 * 输出：
 *      result ：运算结果
 */
module alu(
    input clk,
    input en,
    input [7:0]  opcode,
    input [63:0] operand1,
    input [63:0] operand2,

    output reg [63:0] result
);

// 操作码定义
localparam [7:0]
    OP_ADD  = 8'b0000_0000,
    OP_SUB  = OP_ADD + 1,
    OP_MUL  = OP_SUB + 1,
    OP_DIV  = OP_MUL + 1,

    OP_SLL  = OP_DIV + 1,
    OP_SRL  = OP_SLL + 1,

    OP_AND  = OP_SRL + 1,
    OP_OR   = OP_AND + 1,
    OP_XOR  = OP_OR  + 1;

always @(posedge clk) begin
    if (en) begin
        case(opcode)
            OP_ADD:  result = operand1 + operand2;
            OP_SUB:  result = operand1 - operand2;
            OP_MUL:  result = operand1 * operand2;
            OP_DIV:  result = operand1 / operand2;

            OP_SLL:  result = operand1 << operand2[5:0];  // 移位量取低6位
            OP_SRL:  result = operand1 >> operand2[5:0];
            

            OP_AND:  result = operand1 & operand2;
            OP_OR:   result = operand1 | operand2;
            OP_XOR:  result = operand1 ^ operand2;

            default: result = 64'b0;
        endcase
    end
end

endmodule