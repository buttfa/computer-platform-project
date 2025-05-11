#include "Valu.h"
#include <cstdint>
#include <iostream>

enum ALU_OP {
    ALU_OP_ADD,
    ALU_OP_SUB,
    ALU_OP_MUL,
    ALU_OP_DIV,

    ALU_OP_SLL,
    ALU_OP_SRL,

    ALU_OP_AND,
    ALU_OP_OR,
    ALU_OP_XOR,
};

uint64_t alu_verilog(Valu* alu, uint8_t op, uint64_t a, uint64_t b) {
    // 设置模块输入
    alu->opcode = op;
    alu->operand1 = a;
    alu->operand2 = b;
    alu->en = 1;

    // 仿真时钟触发
    alu->clk = 0;
    alu->eval();
    alu->clk = 1;
    alu->eval();

    return alu->result;
}

uint64_t alu_cpp(uint8_t op, uint64_t a, uint64_t b) {
    switch (op) {
    case ALU_OP_ADD:
        return a + b;
    case ALU_OP_SUB:
        return a - b;
    case ALU_OP_MUL:
        return a * b;
    case ALU_OP_DIV:
        return b != 0 ? a / b : 0;
    case ALU_OP_SLL:
        return a << (b & 0x3F); // 取低6位
    case ALU_OP_SRL:
        return a >> (b & 0x3F); // 取低6位
    case ALU_OP_AND:
        return a & b;
    case ALU_OP_OR:
        return a | b;
    case ALU_OP_XOR:
        return a ^ b;
    default:
        return 0;
    }
}

struct OP {
    uint8_t op;
    uint64_t a;
    uint64_t b;
};

int main() {
    Valu top;
    int test_count = 0, pass_count = 0;

    // 测试用例数组 {op, a, b}
    const OP TestCases[] = {
        {ALU_OP_ADD, 0x1234, 0x5678}, {ALU_OP_SUB, 100, 30},
        {ALU_OP_MUL, 15, 20},         {ALU_OP_DIV, 200, 50},
        {ALU_OP_SLL, 0x1, 4},         {ALU_OP_SRL, 0x80, 3},
        {ALU_OP_AND, 0xFF, 0x0F},     {ALU_OP_OR, 0xF0, 0x0F},
        {ALU_OP_XOR, 0xAA, 0x55}};

    for (const auto& test : TestCases) {
        test_count++;

        // 计算
        uint64_t hw_result = alu_verilog(&top, test.op, test.a, test.b);
        uint64_t sw_result = alu_cpp(test.op, test.a, test.b);

        // 结果比对
        if (hw_result == sw_result) {
            pass_count++;
        } else {
            std::cerr << "FAIL op=" << int(test.op) << " a=0x" << std::hex
                      << test.a << " b=0x" << test.b << " HW=0x" << hw_result
                      << " SW=0x" << sw_result << std::endl;
        }
    }

    std::cout << "Tests completed: " << test_count << " Passed: " << pass_count
              << " (" << (pass_count * 100 / test_count) << "%)" << std::endl;
    return pass_count == test_count ? 0 : 1;
}
