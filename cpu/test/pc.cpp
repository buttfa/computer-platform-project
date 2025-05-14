#include "Vpc.h"
#include "verilated.h"
#include <cstdint>
#include <iostream>

struct TestCase {
    bool reset;        // 复位信号
    bool sign;         // 跳转控制
    uint64_t tar;      // 目标地址
    uint64_t expected; // 预期PC值
};

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vpc dut;
    dut.en = 1;
    int pass_count = 0, total = 0;

    // 测试用例集
    const TestCase tests[] = {// 复位测试
                              {1, 0, 0x0000, 0x0000},
                              // 顺序执行测试
                              {0, 0, 0x0000, 0x0004},
                              {0, 0, 0x0000, 0x0008},
                              // 跳转测试
                              {0, 1, 0x1000, 0x1000},
                              // 跳转后顺序执行
                              {0, 0, 0x0000, 0x1004},
                              // 二次复位
                              {1, 0, 0x0000, 0x0000}};

    for (const auto& t : tests) {
        total++;
        // 设置输入信号
        dut.reset = t.reset;
        dut.sign = t.sign;
        dut.tar = t.tar;

        // 生成时钟脉冲（上升沿）
        dut.clk = 0;
        dut.eval();
        dut.clk = 1;
        dut.eval();

        // 结果验证
        if (dut.pc_addr == t.expected) {
            pass_count++;
        } else {
            std::cout << "FAIL: reset=" << t.reset << " sign=" << t.sign
                      << " tar=0x" << std::hex << t.tar << " got=0x"
                      << dut.pc_addr << " expected=0x" << t.expected
                      << std::endl;
        }
    }

    std::cout << "PC Test: " << pass_count << "/" << total << " pass_count\n";
    return pass_count == total ? 0 : 1;
}