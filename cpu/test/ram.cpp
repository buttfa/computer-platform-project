#include "ram.hpp"
#include "Vram.h"
#include <cstdint>
#include <iostream>
#include <random>

vluint64_t sim_time = 0;

// 初始化随机数引擎
std::random_device rd;
std::mt19937_64 gen(rd());
std::uniform_int_distribution<uint64_t> address_dist(0, (1ULL << 28) -
                                                            1); // 28位地址范围
std::uniform_int_distribution<uint64_t> data_dist; // 默认是64位数据范围

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vram* top = new Vram;

    std::cout << "Starting RAM random test...\n";

    const int num_tests = 3; // 测试次数
    for (int i = 0; i < num_tests; i++) {
        // 生成随机地址和数据
        uint64_t addr = address_dist(gen);
        uint64_t data = data_dist(gen);

        // 写入数据
        ram::write_64bits(top, addr, data);
        std::cout << "Write 0x" << std::hex << data << " to address 0x" << addr
                  << std::dec << std::endl;

        // 读取数据
        uint64_t read_data = ram::read_64bits(top, addr);
        std::cout << "Read 0x" << std::hex << read_data << " from address 0x"
                  << addr << std::dec << std::endl;

        // 检查读取的数据是否与写入的数据匹配
        if (read_data == data) {
            std::cout << "✅ Test " << i << " passed." << std::endl;
        } else {
            std::cerr << "❌ Test " << i << " failed." << std::endl;
        }
    }

    top->final();
    delete top;
    return 0;
}