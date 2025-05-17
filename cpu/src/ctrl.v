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
        /* PREPARE状态：  用于初始化cpu中部件的控制信号 */
        PREPARE = 8'b0,
        /* S1状态：       从Ram中读取指令 */
        S1      = PREPARE+1,
        /* S2状态：       将S1状态中读取的指令写入到IR */
        S2      = S1+1,

        /* ADD_S1状态：   控制alu进行行x[rs1]+x[rs2]的计算 */
        ADD_S1  = S2+1,
        /* ADD_S2状态：   将ADD_S1状态中计算的结果写入到x[rd] */
        ADD_S2  = ADD_S1+1,

        /* ADDI_S1状态：  控制alu进行x[rs1]+setx(imm)的计算 */
        ADDI_S1 = ADD_S2+1,
        /* ADDI_S2状态：  将ADDI_S1状态中计算的结果写入到x[rd] */
        ADDI_S2 = ADDI_S1+1,

        /* SUB_S1状态：   控制alu进行x[rs1]-x[rs2]的计算 */
        SUB_S1  = ADDI_S2+1,
        /* SUB_S2状态：   将SUB_S1状态中计算的结果写入到x[rd] */
        SUB_S2  = SUB_S1+1;

localparam [7:0]
    OP_ADD  = 8'b0000_0000,
    OP_ADDI = OP_ADD + 1,
    OP_SUB  = OP_ADDI + 1,
    OP_MUL  = OP_SUB + 1,
    OP_DIV  = OP_MUL + 1,

    OP_SLL  = OP_DIV + 1,
    OP_SRL  = OP_SLL + 1,

    OP_AND  = OP_SRL + 1,
    OP_OR   = OP_AND + 1,
    OP_NOT  = OP_OR  + 1,
    OP_XOR  = OP_NOT + 1,
    
    OP_LUI  = OP_XOR + 1;

    // 更新状态
    always @(posedge clk) begin
        state <= next_state;
    end

    // 确定下一状态
    always @(*) begin
        case (state)
            PREPARE: next_state = S1;
            S1: next_state = S2;
            S2:
            // 根据指令内容确定之后执行的内容
            // ADDI指令
            if (instr[14:12] == 3'b000 && instr[6:0] == 7'b0010011) begin
                next_state = ADDI_S1;
            end 
            // ADD指令
            else if (instr[31:25] == 7'b0 && instr[14:12] == 3'b0 && instr[6:0] == 7'b0110011) begin
                next_state = ADD_S1;
            end
            else if (instr[31:25] == 7'b0100000 && instr[14:12] == 3'b0 && instr[6:0] == 7'b0110011) begin
                next_state = SUB_S1;
            end
            else begin
                next_state = S1;
            end

            /* ADD指令的状态转移 */
            ADD_S1: next_state = ADD_S2;
            ADD_S2: next_state = S1;

            /* ADDI指令的状态转移 */
            ADDI_S1: next_state = ADDI_S2;
            ADDI_S2: next_state = S1;

            /* SUB指令的状态转移 */
            SUB_S1: next_state = SUB_S2;
            SUB_S2: next_state = S1;
        endcase
    end

    // 执行状态操作
    always @(*) begin
        case (state)
            PREPARE: begin
            end

            S1: begin
                // 所有控制信号的复位
                ram_cs = 1'b0;
                ram_we = 1'b0;
                ram_oe = 1'b0;
                pc_en = 1'b0;
                pc_in_dir = 1'b0;
                pc_sign = 1'b0;
                ir_en = 1'b0;
                reg_en = 1'b0;
                reg_we  = 1'b0;
                reg_in_dir = 1'b0;
                alu_en = 1'b0;
                alu_op  = 8'b0;
                op2_dir = 2'b00;
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

            /* ADD指令 */
            ADD_S1: begin
                // S2状态复位
                ir_en = 1'b0;
                // ADD_S2状态启用
                alu_op = OP_ADD;
                op2_dir = 2'b00;
                alu_en = 1'b1;
            end
            ADD_S2: begin
                // ADD_S2状态启用
                reg_in_dir = 1'b0;
                reg_we = 1'b1;
                reg_en = 1'b1;
                // ADD_S1状态复位
                alu_op = 8'b0;
                op2_dir  = 2'b00;
                alu_en = 1'b0;
            end
            /* ADD指令 */

            /* ADDI指令 */
            ADDI_S1: begin
                // S2状态复位
                ir_en = 1'b0;
                // ADDI_S1状态启用
                alu_op = OP_ADDI;
                op2_dir = 2'b10;
                alu_en = 1'b1;
            end
            ADDI_S2: begin
                // ADDI_S2状态启用
                reg_in_dir = 1'b0;
                reg_we = 1'b1;
                reg_en = 1'b1;
                // ADDI_S1状态复位
                alu_op = 8'b0;
                op2_dir  = 2'b0;
                alu_en = 1'b0;
            end
            /* ADDI指令 */

            /* SUB指令 */
            SUB_S1:  begin
                // S2状态复位
                ir_en = 1'b0;
                // SUB_S1状态启用
                alu_op = OP_SUB;
                op2_dir = 2'b00;
                alu_en = 1'b1;
            end
            SUB_S2: begin
                // SUB_S2状态启用
                reg_in_dir = 1'b0;
                reg_we = 1'b1;
                reg_en = 1'b1;
                // SUB_S1状态复位
                alu_op = 8'b0;
                op2_dir  = 2'b00;
                alu_en = 1'b0;
            end
            /* SUB指令 */
        endcase
    end
    
endmodule