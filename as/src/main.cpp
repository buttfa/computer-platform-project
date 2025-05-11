#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

int reg_idx(const string& r) {
    if (r.size() < 2 || r[0] != 'x')
        throw invalid_argument("非法寄存器名");
    int idx = stoi(r.substr(1));
    if (idx < 0 || idx > 31)
        throw invalid_argument("寄存器编号超出范围");
    return idx;
}

// 写入 32 位大端
void write_uint32_be(ofstream& fout, uint32_t val) {
    for (int i = 0; i < 4; ++i)
        fout.put((val >> (8 * (3 - i))) & 0xFF);
}

// 写入 64 位大端
void write_uint64_be(ofstream& fout, uint64_t val) {
    for (int i = 0; i < 8; ++i)
        fout.put((val >> (8 * (7 - i))) & 0xFF);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr
            << "用法: assembler_bin_output <输出文件> [起始地址, 默认0x1000]\n";
        return 1;
    }

    const char* output_filename = argv[1];
    uint64_t code_start = (argc >= 3) ? strtoull(argv[2], nullptr, 0) : 0x1000;

    ofstream fout(output_filename, ios::binary);
    if (!fout) {
        cerr << "无法打开输出文件: " << output_filename << '\n';
        return 1;
    }

    // 写入头部：分隔符 + 起始地址（大端）
    write_uint32_be(fout, 0);
    write_uint64_be(fout, code_start);

    string line;
    int line_no = 0;
    bool have_prev_addr = false;
    int prev_addr = 0;
    unordered_set<int> used_addrs;

    while (getline(cin, line)) {
        line_no++;
        size_t pos = line.find(';');
        if (pos != string::npos)
            line = line.substr(0, pos);

        auto trim = [](string& s) {
            size_t a = s.find_first_not_of(" \t\r\n");
            if (a == string::npos) {
                s.clear();
                return;
            }
            size_t b = s.find_last_not_of(" \t\r\n");
            s = s.substr(a, b - a + 1);
        };
        trim(line);
        if (line.empty())
            continue;

        vector<string> tok;
        stringstream ss(line);
        string w;
        while (ss >> w)
            tok.push_back(w);

        int curr_addr;
        if (tok[0].size() > 2 && tok[0][0] == '0' && tok[0][1] == 'x') {
            curr_addr = stoi(tok[0], nullptr, 0);
            if (curr_addr % 4 != 0) {
                cerr << "错误：第" << line_no << "行地址未对齐\n";
                return 1;
            }
            if (used_addrs.count(curr_addr)) {
                cerr << "错误：第" << line_no << "行地址重复\n";
                return 1;
            }
            used_addrs.insert(curr_addr);
            tok.erase(tok.begin());
            have_prev_addr = true;
        } else {
            curr_addr = have_prev_addr ? (prev_addr + 4) : 0;
            if (used_addrs.count(curr_addr)) {
                cerr << "错误：第" << line_no << "行地址重复\n";
                return 1;
            }
            used_addrs.insert(curr_addr);
            have_prev_addr = true;
        }
        prev_addr = curr_addr;

        string inst = tok[0];
        uint32_t code = 0;
        try {
            if (inst == "lui") {
                if (tok.size() != 3)
                    throw runtime_error("字段数不匹配");
                int rd = reg_idx(tok[1]);
                int imm = stoi(tok[2], nullptr, 0);
                code = (imm << 12) | (rd << 7) | 0x37;
            } else if (inst == "ld") {
                if (tok.size() != 3)
                    throw runtime_error("字段数不匹配");
                int rd = reg_idx(tok[1]);
                auto p = tok[2].find('('), q = tok[2].find(')');
                if (p == string::npos || q == string::npos)
                    throw runtime_error("格式错误");
                int offset = stoi(tok[2].substr(0, p), nullptr, 0);
                int rs1 = reg_idx(tok[2].substr(p + 1, q - p - 1));
                code =
                    (offset << 20) | (rs1 << 15) | (3 << 12) | (rd << 7) | 0x03;
            } else if (inst == "sd") {
                if (tok.size() != 3)
                    throw runtime_error("字段数不匹配");
                int rs2 = reg_idx(tok[1]);
                auto p = tok[2].find('('), q = tok[2].find(')');
                if (p == string::npos || q == string::npos)
                    throw runtime_error("格式错误");
                int offset = stoi(tok[2].substr(0, p), nullptr, 0);
                int rs1 = reg_idx(tok[2].substr(p + 1, q - p - 1));
                uint32_t imm = offset & 0xFFF;
                uint32_t imm_low = imm & 0x1F, imm_high = (imm >> 5) & 0x7F;
                code = (imm_high << 25) | (rs2 << 20) | (rs1 << 15) |
                       (3 << 12) | (imm_low << 7) | 0x23;
            } else if (inst == "add" || inst == "sub" || inst == "mul" ||
                       inst == "and" || inst == "or" || inst == "xor") {
                if (tok.size() != 4)
                    throw runtime_error("字段数不匹配");
                int rd = reg_idx(tok[1]), rs1 = reg_idx(tok[2]),
                    rs2 = reg_idx(tok[3]);
                int funct3, funct7;
                if (inst == "add") {
                    funct3 = 0;
                    funct7 = 0x00;
                } else if (inst == "sub") {
                    funct3 = 0;
                    funct7 = 0x20;
                } else if (inst == "mul") {
                    funct3 = 0;
                    funct7 = 0x01;
                } else if (inst == "and") {
                    funct3 = 7;
                    funct7 = 0x00;
                } else if (inst == "or") {
                    funct3 = 6;
                    funct7 = 0x00;
                } else if (inst == "xor") {
                    funct3 = 4;
                    funct7 = 0x00;
                }
                code = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) |
                       (funct3 << 12) | (rd << 7) | 0x33;
            } else if (inst == "beq" || inst == "blt" || inst == "bge") {
                if (tok.size() != 4)
                    throw runtime_error("字段数不匹配");
                int rs1 = reg_idx(tok[1]), rs2 = reg_idx(tok[2]),
                    target = stoi(tok[3], nullptr, 0);
                int offset = target - curr_addr;
                if (offset % 2 != 0)
                    throw runtime_error("偏移地址未对齐");
                int imm = offset & 0x1FFF;
                int imm12 = (imm >> 12) & 1, imm11 = (imm >> 11) & 1;
                int imm10_5 = (imm >> 5) & 0x3F, imm4_1 = (imm >> 1) & 0xF;
                int funct3 = (inst == "beq" ? 0 : (inst == "blt" ? 4 : 5));
                code = (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) |
                       (rs1 << 15) | (funct3 << 12) | (imm4_1 << 8) |
                       (imm11 << 7) | 0x63;
            } else if (inst == "jal") {
                if (tok.size() != 2)
                    throw runtime_error("字段数不匹配");
                int offset = stoi(tok[1], nullptr, 0) - curr_addr;
                int imm = offset & 0x1FFFFF;
                int imm20 = (imm >> 20) & 1;
                int imm10_1 = (imm >> 1) & 0x3FF;
                int imm11 = (imm >> 11) & 1;
                int imm19_12 = (imm >> 12) & 0xFF;
                int rd = 1;
                code = (imm20 << 31) | (imm19_12 << 12) | (imm11 << 20) |
                       (imm10_1 << 21) | (rd << 7) | 0x6F;
            } else if (inst == "ret") {
                code = (0 << 20) | (1 << 15) | (0 << 12) | (0 << 7) | 0x67;
            } else if (inst == "jr") {
                if (tok.size() != 2)
                    throw runtime_error("字段数不匹配");
                int imm = stoi(tok[1], nullptr, 0);
                code = (imm << 20) | (0 << 15) | (0 << 12) | (0 << 7) | 0x67;
            } else
                throw runtime_error("未知指令");
        } catch (exception& e) {
            cerr << "错误：第" << line_no << "行：" << e.what() << '\n';
            return 1;
        }

        // 写入指令（大端）
        write_uint32_be(fout, code);
    }

    fout.close();
    cout << "汇编成功，已写入: " << output_filename << '\n';
    return 0;
}