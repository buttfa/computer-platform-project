#include "VPC.h" // Verilator生成的头文件
#include "verilated.h"
#include <cstdint>
#include <iostream>

vluint64_t sim_time = 0;

class PCTester {
  private:
    VPC* dut;       // 被测设备实例
    bool clk_state; // 当前时钟状态

  public:
    PCTester() : dut(new VPC), clk_state(false) {}

    ~PCTester() {
        dut->final();
        delete dut;
    }

    // 生成时钟边沿
    void tick() {
        clk_state = !clk_state;
        dut->clk = clk_state;
        dut->eval();
        sim_time += 5; // 模拟时间推进
    }

    // 执行完整时钟周期
    void cycle() {
        tick(); // 上升沿
        tick(); // 下降沿
    }

    // 运行测试流程
    void run() {
        std::cout << "===== PC Test Start =====" << std::endl;

        // 测试1：复位功能
        dut->reset = 1;
        cycle(); // 复位需要至少一个周期
        std::cout << "[Reset] PC: 0x" << std::hex << dut->pc
                  << " (Expected: 0x0)\n";
        dut->reset = 0;

        // 测试2：顺序执行
        dut->sign = 0;
        for (int i = 0; i < 3; ++i) {
            cycle();
            std::cout << "[Seq " << i + 1 << "] PC: 0x" << dut->pc
                      << " (Expected: 0x" << (i + 1) * 4 << ")\n";
        }

        // 测试3：跳转功能
        const uint64_t target = 0x1000;
        dut->sign = 1;
        dut->tar = target;
        cycle();
        std::cout << "[Jump] PC: 0x" << dut->pc << " (Expected: 0x" << target
                  << ")\n";

        // 测试4：跳转后继续顺序
        dut->sign = 0;
        cycle();
        std::cout << "[PostJump] PC: 0x" << dut->pc << " (Expected: 0x"
                  << target + 4 << ")\n";

        std::cout << "===== PC Test Complete =====" << std::endl;
    }
};

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    PCTester tester;
    tester.run();
    return 0;
}