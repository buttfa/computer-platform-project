module cpu (
    input clk,
    input reset,

    inout [63:0] bus_data, // 数据总线
    output [63:0] bus_addr, // 地址总线
    output ram_cs, // ram的使能信号
    output ram_we, // ram的写使能信号
    output ram_oe // ram的读使能信号
);

    // 程序计数器相关
    wire pc_en;
    wire [1:0] pc_in_dir;
    wire pc_sign;
    wire [63:0] pc_addr;

    // 指令寄存器相关
    wire ir_en;
    wire [31:0] instr_raw;
    
    // 寄存器文件相关
    wire reg_en;
    wire [4:0] rs1, rs2, rd; // 寄存器索引
    wire [63:0] reg_data1, reg_data2; // rs1和rs2的值
    wire reg_we;
    wire [1:0] reg_in_dir;
    
    // ALU相关
    wire alu_en;
    wire [63:0] alu_result;
    wire [7:0] alu_op;
    wire [1:0] op2_dir;
    wire alu_zero;

    pc pc_inst(
        .clk(clk),
        .en(pc_en),
        .reset(1'b0),
        .tar(
            // jal
            pc_in_dir==2'b01 ? pc_addr+{{44{instr_raw[31]}}, instr_raw[31:31], instr_raw[19:12], instr_raw[20:20], instr_raw[30:21]} :
            // jalr
            pc_in_dir==2'b10 ? reg_data1+{{52{instr_raw[31]}}, instr_raw[31:20]} : 
            // beq
            pc_in_dir==2'b00 && alu_result == 64'b0 ? pc_addr+{{52{instr_raw[31]}}, {instr_raw[31],instr_raw[7],instr_raw[30:25],instr_raw[11:8]}} :
            // bge
            pc_in_dir==2'b11 && alu_result[63] == 1'b0 ? pc_addr+{{52{instr_raw[31]}}, {instr_raw[31],instr_raw[7],instr_raw[30:25],instr_raw[11:8]}} :
            pc_addr + 64'b0
        ),
        .sign(pc_sign),
        .pc_addr(pc_addr)
    );

    ir ir_inst(
        .en(ir_en),
        .instr_in(bus_data[63:32]), // 将bus_data的高32位作为指令缓存
        .instr_out(instr_raw)
    );

    regfile regfile_inst(
        .en(reg_en),
        
        .rd(instr_raw[11:7]), // 寄存器索引
        .rs1(instr_raw[19:15]),
        .rs2(instr_raw[24:20]),

        .data1(reg_data1), // 输出rs1和rs2寄存器的值
        .data2(reg_data2),

        .we(reg_we), // 写输入数据到rd寄存器
        // rd寄存器的值，只可能来自ALU/RAM/PC
        .write_data(reg_in_dir==2'b01 ? bus_data : 
                    reg_in_dir==2'b10 ? alu_result :
                    reg_in_dir==2'b11 ? pc_addr :
                    64'bZ
                    )
    );

    // 向数据总线写数据，ram信号由controller控制
    assign bus_addr = 
    // 从ram读取pc地址指向的指令到ir
    (ram_oe && pc_en) ? pc_addr : 
    
    // 从ram的x[rs1]+sign-extend(offset)地址出读取8个字节的数据到x[rd]   ld指令
    (ram_oe) ? reg_data1+{{52{instr_raw[31]}}, instr_raw[31:20]} : 
    
    // 将x[rs2]写入ram的x[rs1]+sign-extend(offset)地址   sd指令
    (ram_we) ? reg_data1+{{52{instr_raw[31]}}, instr_raw[31:25], instr_raw[11:7]} : 
    64'bZ;
    assign bus_data = (ram_we) ? reg_data2 : 64'bZ;

    // alu 只对来自寄存器的数据/立即数进行运算
    alu alu_inst(
        .en(alu_en),
        .opcode(alu_op),
        .operand1(reg_data1), // 操作数1,只会是寄存器rs1的值
        .operand2((op2_dir == 2'b00) ? reg_data2 :
                  // 来自 lui
                  (op2_dir == 2'b01) ? {{44{instr_raw[31]}}, instr_raw[31:12]} :
                  // 来自 addi
                  (op2_dir == 2'b10) ? {{52{instr_raw[31]}}, instr_raw[31:20]} : 
                  64'bZ), // 操作数2,可能是寄存器rs2的值，也可能是立即数
        .result(alu_result)
    );

    ctrl ctrl_inst(
        .clk(clk),
        // 根据instr_raw，控制各个模块的使能信号
        .instr(instr_raw),

        // ram的控制信号
        .ram_cs(ram_cs),
        .ram_we(ram_we),
        .ram_oe(ram_oe),

        // pc的控制信号
        .pc_en(pc_en),
        .pc_in_dir(pc_in_dir),
        .pc_sign(pc_sign),

        // ir的控制信号
        .ir_en(ir_en),

        // regfile的控制信号
        .reg_en(reg_en),
        .reg_we(reg_we),
        .reg_in_dir(reg_in_dir),

        // alu的控制信号
        .alu_en(alu_en),
        .alu_op(alu_op),
        .op2_dir(op2_dir)
    );
endmodule