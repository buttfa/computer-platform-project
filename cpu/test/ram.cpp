#include "ram.hpp"
#include "Vram.h"
#include <iostream>

vluint64_t sim_time = 0;

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vram* top = new Vram;

    std::cout << "Starting RAM test...\n";

    // 模拟写入数据到地址 0x27
    ram::write_64bits(top, 0x27, 0x00001001);
    std::cout << "Wrote 0x00001001 to address 0x27\n";

    uint64_t read_data = ram::read_64bits(top, 0x27);
    std::cout << "Read from address 0x27: 0x" << std::hex << read_data << "\n";

    if (read_data == 0x00001001) {
        std::cout << "✅ Test passed.\n";
    } else {
        std::cerr << "❌ Test failed.\n";
    }

    top->final();
    delete top;
    return 0;
}