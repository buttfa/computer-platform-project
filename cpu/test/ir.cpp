#include "Vir.h"
#include <cassert>
#include <verilated.h>

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vir* top = new Vir;

    // 初始化测试
    top->clk = 0;
    top->en = 0;
    top->instr_in = 0;
    top->eval();

    /**************** 测试用例组 ****************/

    // 测试1：初始状态验证
    printf("Test 1: Initial State\n");
    top->eval();
    assert(top->instr_out == 0 && "Initial output not zero");
    printf("Test 1 Passed\n");

    // 测试2：使能信号有效时的数据锁存
    printf("\nTest 2: Enabled Operation\n");
    top->en = 1;
    top->instr_in = 0x12345678;
    top->clk = 1; // 上升沿触发
    top->eval();
    assert(top->instr_out == 0x12345678 && "Latching failed when enabled");
    printf("Test 2 Passed\n");

    // 测试3：使能信号无效时的数据保持
    printf("\nTest 3: Disabled State\n");
    top->en = 0;
    top->instr_in = 0xFFFF0000; // 改变输入
    top->clk = 0;
    top->eval();
    top->clk = 1; // 上升沿触发
    top->eval();
    assert(top->instr_out == 0x12345678 && "Data changed when disabled");
    printf("Test 3 Passed\n");

    // 测试4：多周期操作验证
    printf("\nTest 4: Multi-cycle Operation\n");
    top->en = 1;
    for (uint32_t i = 1; i <= 3; i++) {
        top->instr_in = i; // 高位数据变化
        top->clk = 0;
        top->eval();
        top->clk = 1; // 上升沿触发
        top->eval();
        assert(top->instr_out == i && "Multi-cycle operation failed");
    }
    printf("Test 4 Passed\n");

    /**************** 清理操作 ****************/
    top->final();
    delete top;
    return 0;
}