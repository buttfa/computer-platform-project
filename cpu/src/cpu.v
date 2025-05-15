module cpu (
    input clk,
    input reset,

    inout [63:0] bus_data, // 数据总线
    output [63:0] bus_addr, // 地址总线
    output ram_cs, // ram的使能信号
    output ram_we, // ram的写使能信号
    output ram_oe // ram的读使能信号
);
    // 指令数据
    wire [31:0] instr_raw;
    
    // 寄存器文件相关
    wire [4:0] rs1, rs2, rd; // 寄存器索引
    wire [63:0] reg_data1, reg_data2; // rs1和rs2的值
    wire reg_write_en;
    
    // ALU相关
    wire [63:0] alu_result;
    wire [3:0] alu_op;
    wire alu_zero;
    
    // 立即数生成
    wire [63:0] imm_value;
    
    // 控制信号
    wire branch;
    wire reg_in_dir;
    wire alu_src;
    wire [2:0] imm_sel;

    pc pc_inst(
        .clk(clk),
        .en(pc_en),
        .reset(reset),
        .tar(pc_in_dir==1'b0 && instr_raw[31]==1'b0 ? pc_addr+{44'b0, instr_raw[31:12]} :
            pc_in_dir==1'b0 && instr_raw[31]==1'b1 ? pc_addr+{44{1'b1}, instr_raw[31:12]} :

            pc_in_dir==1'b1 && instr_raw[31]==1'b0 ? reg_data1+{44'b0, instr_raw[31:20]} : 
            pc_in_dir==1'b1 && instr_raw[31]==1'b1 ? reg_data1+{44{1'b1}, instr_raw[31:20]}
        ),
        .sign(pc_sign),
        .pc(pc_addr)
    );

    ir ir_inst(
        .clk(clk),
        .en(ir_en),
        .instr_in(bus_data[63:32]), // 将bus_data的高32位作为指令输出
        .instr_out(instr_raw)
    );

    control_unit ctrl_inst(
        .clk(clk),
        // 根据instr_raw，控制各个模块的使能信号
        .instr(instr_raw),

        .branch(branch),

        // ram的控制信号
        .ram_cs(ram_cs),
        .ram_we(ram_we),
        .ram_oe(ram_oe),

        .reg_in_dir(reg_in_dir),
        .reg_write(reg_write_en),
        .alu_op(alu_op),
        .alu_src(alu_src),
        .imm_sel(imm_sel)
    );

    regfile regfile_inst(
        .clk(clk),
        .en(reg_en),
        
        .rd(instr_raw[11:7]), // 寄存器索引
        .rs1(instr_raw[19:15]),
        .rs2(instr_raw[24:20]),

        .data1(reg_data1), // 输出rs1和rs2寄存器的值
        .data2(reg_data2),

        .we(reg_write_en), // 写输入数据到rd寄存器
        // rd寄存器的值，只可能来自ALU/RAM
        .write_data(reg_in_dir ? bus_data : alu_result)
    );

    // 向数据总线写数据，ram信号由controller控制
    assign bus_addr = (ram_cs && ram_oe && pc_en) ? pc_addr : 
                    (ram_cs && ram_oe && reg_en) ? reg_addr1+{instr_raw[31:25], instr_raw[11:7]} : 64'bZ;
    assign bus_data = (ram_cs && ram_we) ? reg_data2 : 64'bZ;

    // alu 只对来自寄存器的数据/立即数进行运算
    alu alu_inst(
        .clk(clk),
        .en(alu_en),
        .opcode(alu_op),
        .operand1(reg_data1), // 操作数1,只会是寄存器rs1的值
        .operand2(reg_data2_en ? reg_data2 :
                  // 来自 lui
                  imm20_en ? instr_raw[31:12] :
                  // 来自 addi
                  imm12_en ? instr_raw[31:20] : 64'Z), // 操作数2,可能是寄存器rs2的值，也可能是立即数
        .result(alu_result)
    );
endmodule