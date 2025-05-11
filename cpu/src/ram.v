/*
 * 模块：RAM模块
 * 简述：提供 256M x 8bit 的RAM模块，支持读写操作，使用大端序存储。
 * 输入：
 *      clk  ：时钟信号
 *      cs   ：片选信号（0使能）
 *      we   ：写使能信号（0使能）
 *      oe   ：读使能信号（0使能）
 *      addr ：地址总线，保持64bit输入，但实际使用低28位（256M地址空间）
 *      data ：数据总线，64位
 * 输出：
 *      data ：数据总线，64位
 */
module ram (
    input        clk,
    input        cs,      
    input        we,     
    input        oe,     
    input  [63:0] addr,   

    inout  [63:0] data   
);

    /* 256M x 8bit 的存储空间，即256MB */
    reg [7:0] mem [0:268435455];

    /* 内部寄存器，用于存储要读取的数据 */
    reg [63:0] data_reg;

    // 数据总线控制:  当OE为低（读），则从data_reg中驱动数据
    //              当WE为低（写），则将数据捕获到内存
    always @(posedge clk) begin
        if (!cs) begin
            if (!we) begin
                mem[addr[27:0]+0] <= data[63:56];
                mem[addr[27:0]+1] <= data[55:48];
                mem[addr[27:0]+2] <= data[47:40];
                mem[addr[27:0]+3] <= data[39:32];
                mem[addr[27:0]+4] <= data[31:24];
                mem[addr[27:0]+5] <= data[23:16];
                mem[addr[27:0]+6] <= data[15:8];
                mem[addr[27:0]+7] <= data[7:0];  
            end
            if (!oe)
                data_reg <= {mem[addr[27:0]+0], mem[addr[27:0]+1], mem[addr[27:0]+2], mem[addr[27:0]+3], mem[addr[27:0]+4], mem[addr[27:0]+5], mem[addr[27:0]+6], mem[addr[27:0]+7]};  // Read operation
        end
    end

    // 双向数据总线的三态缓冲器
    assign data = (!oe) ? data_reg : 64'bz;

endmodule