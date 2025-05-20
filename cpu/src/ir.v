/*
 * 模块：IR指令寄存器
 * 简述：指令寄存器，将从RAM中读出的指令写入并暂存于该模块，并输出给控制单元和其他部件。
 * 输入：
 *      en        ：使能信号（1使能）
 *      instr_in  ：指令输入，32位
 * 输出：
 *      instr_out ：指令输出，32位
 */
module ir (
    input en,
    input [31:0] instr_in,
    output [31:0] instr_out
);
    // 用于在内部存储指令
    reg [31:0] instr;

    always @(posedge en) begin
        // 在使能信号的上升沿时，更新指令
        instr = instr_in;
    end

    // 输出指令
    assign instr_out = instr;
    
endmodule