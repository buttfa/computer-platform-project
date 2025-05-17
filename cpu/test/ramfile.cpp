#include "Vramfile.h"
#include "verilated.h"
#include <cstdint>
#include <iostream>
#include <random>

vluint64_t sim_time = 0;

// 大端字节序转换函数（与Verilog模块一致）
uint64_t convert(uint64_t data) {
    return ((data & 0xFF00000000000000ULL) >> 56) |
           ((data & 0x00FF000000000000ULL) >> 40) |
           ((data & 0x0000FF0000000000ULL) >> 24) |
           ((data & 0x000000FF00000000ULL) >> 8) |
           ((data & 0x00000000FF000000ULL) << 8) |
           ((data & 0x0000000000FF0000ULL) << 24) |
           ((data & 0x000000000000FF00ULL) << 40) |
           ((data & 0x00000000000000FFULL) << 56);
}

// 封装RAM操作函数
namespace ramfile {
void write_64bits(Vramfile* ram, uint64_t addr, uint64_t data) {
    ram->mem_write = 1;
    ram->addr = addr;
    ram->data_in = convert(data);

    // 生成时钟上升沿
    ram->clk = 0;
    ram->eval();
    ram->clk = 1;
    ram->eval();
    sim_time += 10;
}

uint64_t read_64bits(Vramfile* ram, uint64_t addr) {
    ram->mem_write = 0;
    ram->addr = addr;
    ram->eval();
    return convert(ram->data_out);
}
} // namespace ramfile

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vramfile* top = new Vramfile;
    std::cout << "RAM Random Test Start (28-bit address space)\n";

    // 初始化随机数引擎
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> addr_dist(0, (1ULL << 28) - 1);
    std::uniform_int_distribution<uint64_t> data_dist;

    const int num_tests = 3;
    int passed = 0;

    for (int i = 0; i < num_tests; ++i) {
        // 生成测试数据
        uint64_t addr = addr_dist(gen) & 0x0FFFFFFF; // 确保28位地址
        uint64_t data = data_dist(gen);

        // 写入操作
        ramfile::write_64bits(top, addr, data);
        std::cout << "Write 0x" << std::hex << data << " to address 0x" << addr
                  << std::dec << std::endl;

        // 读取操作
        uint64_t read_data = ramfile::read_64bits(top, addr);
        std::cout << "Read 0x" << std::hex << read_data << " from address 0x"
                  << addr << std::dec << std::endl;

        // 结果验证
        if (read_data == data) {
            std::cout << "? Test " << i << " passed.\n";
            passed++;
        } else {
            std::cerr << "? Test " << i << " failed: " << "Expected 0x"
                      << std::hex << data << " Got 0x" << read_data << "\n";
        }
    }

    std::cout << "\nTest: " << passed << "/" << num_tests << " passed ("
              << (passed * 100 / num_tests) << "%)\n";

    top->final();
    delete top;
    return 0;
}