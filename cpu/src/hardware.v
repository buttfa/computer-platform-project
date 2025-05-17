module hardware (
    input clk,

    input test_clk,
    input test_en,
    input test_cs,
    input test_we,
    input test_oe,
    input [63:0] test_addr,
    input [63:0] test_data
);

    wire [63:0] bus_addr;
    wire [63:0] bus_data;
    wire ram_cs, ram_we, ram_oe;
    wire [63:0] ram_data;  // 中间信号

    cpu cpu_inst (
        .clk(clk),
        .reset(1'b0),
        .bus_addr(bus_addr),
        .bus_data(ram_data),
        .ram_cs(ram_cs),
        .ram_we(ram_we),
        .ram_oe(ram_oe)
    );

    ram ram_inst (
        .cs(test_en ? test_cs : ram_cs),
        .we(test_en ? test_we : ram_we),
        .oe(test_en ? test_oe : ram_oe),
        .addr(test_en ? test_addr : bus_addr),
        .data(ram_data)  // 使用中间信号
    );

    // 三元运算的结果赋值给中间信号
    assign ram_data = test_en ? test_data : bus_data;

endmodule