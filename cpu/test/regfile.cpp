#include "Vregfile.h"
#include <cassert>
#include <cstdio>
#include <verilated.h>

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vregfile* top = new Vregfile;

    // 初始化信号
    top->en = 0;
    top->we = 0;
    top->rd = 0;
    top->rs1 = 0;
    top->rs2 = 0;
    top->write_data = 0;
    top->eval();

    /**************** 基础功能测试组 ****************/

    // 测试1：x0寄存器恒为0
    printf("Test 1: x0 Hardwired Zero\n");
    top->rs1 = 0; // 读取x0
    top->eval();
    assert(top->data1 == 0 && "x0 not zero");
    printf("Test 1 Passed\n");

    // 测试2：寄存器写入与读取
    printf("\nTest 2: Basic Write/Read\n");
    top->we = 1;
    top->rd = 5; // 写x5
    top->write_data = 0x123456789ABCDEF0;
    top->en = 0;
    top->eval();
    top->en = 1; // 触发写入
    top->eval();
    top->en = 0;

    top->rs1 = 5; // 读x5
    top->eval();
    assert(top->data1 == 0x123456789ABCDEF0 && "Write x5 failed");
    printf("Test 2 Passed\n");

    // 测试3：x0写保护
    printf("\nTest 3: x0 Write Protection\n");
    top->rd = 0; // 尝试写x0
    top->write_data = 0xDEADBEEF;
    top->en = 1; // 触发写入
    top->eval();
    top->en = 0;

    top->rs1 = 0;
    top->eval();
    assert(top->data1 == 0 && "x0 modified");
    printf("Test 3 Passed\n");

    // 测试4：写使能无效测试
    printf("\nTest 4: Write Enable Disabled\n");
    top->we = 0; // 写使能关闭
    top->rd = 10;
    top->write_data = 0x5555AAAA;
    top->en = 1; // 触发但we=0
    top->eval();
    top->en = 0;

    top->rs1 = 10;
    top->eval();
    assert(top->data1 != 0x5555AAAA && "Write occurred when disabled");
    printf("Test 4 Passed\n");

    /**************** 高级功能测试组 ****************/

    // 测试5：多寄存器操作
    printf("\nTest 5: Multi-register Operation\n");
    for (int i = 1; i <= 3; i++) {
        top->we = 1;
        top->rd = i;
        top->write_data = i * 0x1000;
        top->en = 1; // 触发写入
        top->eval();
        top->en = 0;

        top->rs1 = i;
        top->eval();
        assert(top->data1 == i * 0x1000 && "Multi-reg write failed");
    }
    printf("Test 5 Passed\n");

    // 测试6：异步读测试
    printf("\nTest 6: Asynchronous Read\n");
    top->we = 1;
    top->rd = 7;
    top->write_data = 0x76543210;
    top->en = 1; // 写入x7
    top->eval();
    top->rs1 = 7; // 立即读取
    top->eval();
    assert(top->data1 == 0x76543210 && "Async read failed");
    printf("Test 6 Passed\n");

    /**************** 清理操作 ****************/
    top->final();
    delete top;
    printf("\nAll Tests Passed!\n");
    return 0;
}