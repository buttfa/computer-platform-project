module ctrl (
    input clk,
    input [31:0] instr,

    output reg ram_cs,
    output reg ram_we,
    output reg ram_oe,

    output reg pc_en,
    output reg pc_in_dir,
    output reg pc_sign,

    output reg ir_en,

    output reg reg_en,
    output reg reg_we,
    output reg reg_in_dir,

    output reg alu_en,
    output reg [7:0] alu_op,
    output reg [1:0] op2_dir
);
    reg [7:0] state;
    reg [7:0] next_state;

    parameter 
        PREPARE = 8'b0,
        S1      = PREPARE+1,
        S2      = S1+1;

    // 更新状态
    always @(posedge clk) begin
        state <= next_state;
    end

    // 确定下一状态
    always @(*) begin
        case (state)
            PREPARE: next_state = S1;
            S1: next_state = S2;
            S2: next_state = S1;
        endcase
    end

    // 执行状态操作
    always @(*) begin
        case (state)
            // PERPARE: 准备状态
            PREPARE: begin
            end

            // S1: 读指令并且PC+=4
            S1: begin
                // 所有状态复位
                ram_cs = 1'b0;
                ram_oe = 1'b0;
                pc_en = 1'b0;
                ir_en = 1'b0;

                // S1状态启用
                ram_cs = 1'b1;
                ram_oe = 1'b1;
                pc_en = 1'b1;
            end
            S2:  begin
                // S1状态复位
                ram_cs = 1'b0;
                ram_oe = 1'b0;
                pc_en = 1'b0;

                // S2状态启用
                ir_en = 1'b1;
            end
        endcase
    end
    
endmodule