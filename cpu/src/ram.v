/*
 * 模块：RAM模块
 * 简述：提供 256M x 8bit 的RAM模块，支持读写操作，使用大端序存储。
 * 输入：
 *      cs   ：片选信号（1使能）
 *      we   ：写使能信号（1使能）
 *      oe   ：读使能信号（1使能）
 *      addr ：地址总线，保持64bit输入，但实际使用低28位（256M地址空间）
 *      data ：数据总线，64位
 * 输出：
 *      data ：数据总线，64位
 */
module ram (
    input        cs,      
    input        we,     
    input        oe,     
    input  [63:0] addr,   

    inout  [63:0] data   
);

    /* 256M x 8bit 的存储空间，即256MB */
    reg [7:0] mem [0:268435455];
    
    // 三态控制逻辑
    reg [63:0] data_out;
    reg data_dir; // 0: input(z), 1: output
    
    // 大端序实现
    always @(posedge cs) begin
        if (we) begin
            mem[addr[27:0]+0] <= data[63:56];
            mem[addr[27:0]+1] <= data[55:48];
            mem[addr[27:0]+2] <= data[47:40];
            mem[addr[27:0]+3] <= data[39:32];
            mem[addr[27:0]+4] <= data[31:24];
            mem[addr[27:0]+5] <= data[23:16];
            mem[addr[27:0]+6] <= data[15:8];
            mem[addr[27:0]+7] <= data[7:0];
            data_dir <= 0;
        end
        else if (oe) begin
            data_out <= {mem[addr[27:0]+0], mem[addr[27:0]+1],
                        mem[addr[27:0]+2], mem[addr[27:0]+3],
                        mem[addr[27:0]+4], mem[addr[27:0]+5],
                        mem[addr[27:0]+6], mem[addr[27:0]+7]};
            data_dir <= 1;
        end
    end
    
    assign data = data_dir ? data_out : 64'bz;

endmodule