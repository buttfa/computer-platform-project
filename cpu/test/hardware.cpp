#include "hardware.hpp"
#include "Vhardware.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <verilated_vcd_c.h>

int main(int argc, char** argv) {
    Verilated::traceEverOn(true); // 开启波形跟踪
    Vhardware hardware;
    VerilatedVcdC trace;       // 实例化波形跟踪对象
    hardware.trace(&trace, 5); // 将波形跟踪对象与仿真模型关联
#ifndef __HARDWARE_RELEASE__
    trace.open("./sim/hardware.vcd"); // 打开波形文件
#else
    trace.open("./cpu/sim/hardware.vcd"); // 打开波形文件
#endif

#ifndef __HARDWARE_RELEASE__
    uint64_t instr = 0;
    /* addi x1 x1 1 ; x1==1 */
    instr = (uint64_t)0b00000000000100001000000010010011 << 32;
    hardware::write_64bits(&hardware, 0x00, instr);
    /* add x2 x1 x1 ; x2==2*/
    instr = (uint64_t)0b00000000000100001000000100110011 << 32;
    hardware::write_64bits(&hardware, 0x04, instr);
    /* sub x3 x2 x1 ; x3==1*/
    instr = (uint64_t)0b01000000000100010000000110110011 << 32;
    hardware::write_64bits(&hardware, 0x08, instr);
    /* mul x3 x2 x2 ; x3==4*/
    instr = (uint64_t)0b00000010001000010000000110110011 << 32;
    hardware::write_64bits(&hardware, 0x0C, instr);
    /* div x4 x3 x2 ; x4==2*/
    instr = (uint64_t)0b00000010001000011100001000110011 << 32;
    hardware::write_64bits(&hardware, 0x10, instr);
    /* sll x1 x1 x1 ; x1==2*/
    instr = (uint64_t)0b00000000000100001001000010110011 << 32;
    hardware::write_64bits(&hardware, 0x14, instr);
    /* srl x1 x1 x1 ; x1==0*/
    instr = (uint64_t)0b00000000000100001101000010110011 << 32;
    hardware::write_64bits(&hardware, 0x18, instr);
    /* lui x1 0x0100_0000 ; x1==0x0100_0000 (imm[31:12]==0x01000) */
    instr = (uint64_t)0b00000001000000000000000010110111 << 32;
    hardware::write_64bits(&hardware, 0x1C, instr);
    /* or x1 x3 x4 ; x1==6*/
    instr = (uint64_t)0b00000000010000011110000010110011 << 32;
    hardware::write_64bits(&hardware, 0x20, instr);
    /* and x1 x1 x4 ; x1==2*/
    instr = (uint64_t)0b00000000010000001111000010110011 << 32;
    hardware::write_64bits(&hardware, 0x24, instr);
    /* xor x1 x1 x1 ; x1==0*/
    instr = (uint64_t)0b00000000000100001100000010110011 << 32;
    hardware::write_64bits(&hardware, 0x28, instr);
    /* ld x1 x1 0x0 ; x1==0x0010_8093_0010_8133 */
    instr = (uint64_t)0b00000000000000001011000010000011 << 32;
    hardware::write_64bits(&hardware, 0x2C, instr);
    /* sd x4 x0 0x0 ; [0+0]==2 */
    instr = (uint64_t)0b00000000010000000011000000100011 << 32;
    hardware::write_64bits(&hardware, 0x30, instr);
    /* ld x1 x0 0x0 ; x1==2 */
    instr = (uint64_t)0b00000000000000000011000010000011 << 32;
    hardware::write_64bits(&hardware, 0x34, instr);
    // /* beq x1 x2 0xFFC ; x1==x2==2，所以原地循环(0xFFC是12位的-4补码) */
    // instr = (uint64_t)0b11111110001000001000110011100011 << 32;
    // hardware::write_64bits(&hardware, 0x38, instr);
    /* bge x1 x2 0xFFC ; x1==x2==2，所以原地循环(0xFFC是12位的-4补码) */
    // instr = (uint64_t)0b11111110001000001101110011100011 << 32;
    // hardware::write_64bits(&hardware, 0x38, instr);
    // /* jal x1 0xFFFFC;
    //  * x1==0x3C(下一条指令的地址)，原地循环(OxFFFC是20位的-4补码) */
    // instr = (uint64_t)0b11111111100111111111000011101111 << 32;
    // hardware::write_64bits(&hardware, 0x38, instr);
    /* jalr x1 x0 0x38; x1==0x3C(下条指令的地址), pc_addr==0x38 */
    // instr = (uint64_t)0b00000011100000000010000011100111 << 32;
    // hardware::write_64bits(&hardware, 0x38, instr);
    /* xori x1 x1 0xFFF ; x1==0xFFFF_FFFF_FFFF_FFFD，其中0xFFF是12位的-1补码 */
    instr = (uint64_t)0b11111111111100001100000010010011 << 32;
    hardware::write_64bits(&hardware, 0x38, instr);
#else
    // 检查格式
    if (argc != 2) {
        cerr << "Usage: Vhardware <bin_file>" << endl;
        return 1;
    }

    // 打开文件
    string bin_file = argv[1];
    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    // 读取文件并将32位指令一次写入硬件
    uint32_t address = 0x00;
    while (true) {
        // 检测文件是否结束
        if (file.eof())
            break;

        // 读取32位指令
        uint32_t instruction;
        file.read(reinterpret_cast<char*>(&instruction), sizeof(instruction));
        // 转化为大端序
        uint32_t new_instr = (instruction & 0x000000FF) >> 0 << 24 |
                             (instruction & 0x0000FF00) >> 8 << 16 |
                             (instruction & 0x00FF0000) >> 16 << 8 |
                             (instruction & 0xFF000000) >> 24 << 0;
        instruction = new_instr;

        // 检测文件是否结束
        if (file.eof())
            break;

        // 检测文件是否读取错误
        if (!file) {
            std::cerr << "Error reading file at address 0x" << std::hex
                      << address << std::endl;
            return 1;
        }

        // 将32位指令扩展到64位高位
        uint64_t full_instruction = static_cast<uint64_t>(instruction) << 32;
        hardware::write_64bits(&hardware, address, full_instruction);
        address += 4; // 地址按4字节步进
    }
    file.close();
#endif

    hardware.clk = 1;
    for (int i = 0; i < 200; i++) {
        hardware.clk = !hardware.clk;
        hardware.eval();
        trace.dump(i);
    }

    trace.close();
}