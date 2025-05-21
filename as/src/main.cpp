#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
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

void write_uint32_be(ofstream& fout, uint32_t val) {
    for (int i = 0; i < 4; ++i)
        fout.put((val >> (8 * (3 - i))) & 0xFF);
}

void write_uint64_be(ofstream& fout, uint64_t val) {
    for (int i = 0; i < 8; ++i)
        fout.put((val >> (8 * (7 - i))) & 0xFF);
}

vector<string> tokenize(string line) {
    vector<string> res;
    stringstream ss(line);
    string tok;
    while (ss >> tok)
        res.push_back(tok);
    return res;
}

bool is_hex_addr(const string& s) {
    return s.size() >= 3 && s[0] == '0' && s[1] == 'x';
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: assembler <output_file> [start_addr, 默认0x1000]\n";
        return 1;
    }

    const char* output_file = argv[1];
    uint64_t code_start = (argc >= 3) ? strtoull(argv[2], nullptr, 0) : 0x1000;

    // First pass
    unordered_map<string, int> label_addr;
    vector<pair<int, vector<string>>> program;
    unordered_set<int> used_addrs;
    string line;
    int line_no = 0;
    int curr_addr = 0;
    bool has_prev = false;

    while (getline(cin, line)) {
        ++line_no;
        size_t cpos = line.find(';');
        if (cpos != string::npos)
            line = line.substr(0, cpos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty())
            continue;

        vector<string> tok = tokenize(line);
        if (tok.empty())
            continue;

        if (tok[0].back() == ':') {
            string label = tok[0].substr(0, tok[0].size() - 1);
            label_addr[label] = curr_addr;
            tok.erase(tok.begin());
            if (tok.empty())
                continue;
        }

        if (is_hex_addr(tok[0])) {
            curr_addr = stoi(tok[0], nullptr, 0);
            tok.erase(tok.begin());
        } else if (!has_prev) {
            curr_addr = 0;
        } else {
            curr_addr += 4;
        }

        if (used_addrs.count(curr_addr)) {
            cerr << "错误：地址重复：" << hex << curr_addr << '\n';
            return 1;
        }
        used_addrs.insert(curr_addr);
        has_prev = true;
        program.emplace_back(curr_addr, tok);
    }

    // Second pass
    ofstream fout(output_file, ios::binary);
    if (!fout) {
        cerr << "无法打开输出文件: " << output_file << '\n';
        return 1;
    }

    //   write_uint32_be(fout, 0);          // 分隔符
    //   write_uint64_be(fout, code_start); // 起始地址

    for (const auto& [addr, tok_orig] : program) {
        vector<string> tok = tok_orig;
        string inst = tok[0];
        uint32_t code = 0;

        try {
            // 伪指令处理
            if (inst == "not") {
                if (tok.size() != 3)
                    throw runtime_error("not 伪指令格式错误");
                tok = {"xori", tok[1], tok[2], "-1"};
                inst = "xori";
            }

            if (inst == "ret") {
                code = (0 << 20) | (1 << 15) | (0 << 12) | (0 << 7) | 0x67;
                write_uint32_be(fout, code);
                continue;
            }

            if (inst == "blt")
                throw runtime_error("不支持 blt 指令");

            // 标签替换
            if ((inst == "beq" || inst == "jal" || inst == "jr") &&
                !isdigit(tok.back()[0]) && tok.back()[0] != '-') {
                string label = tok.back();
                if (!label_addr.count(label))
                    throw runtime_error("未定义标签: " + label);
                int offset = label_addr[label] - addr;
                tok.back() = to_string(offset);
            }

            if (inst == "lui") {
                if (tok.size() != 3)
                    throw runtime_error("lui 参数数量错误");
                int rd = reg_idx(tok[1]);
                int imm = stoi(tok[2], nullptr, 0);
                code = (imm << 12) | (rd << 7) | 0x37;
            } else if (inst == "ld") {
                if (tok.size() != 3)
                    throw runtime_error("ld 参数数量错误");
                int rd = reg_idx(tok[1]);
                auto p = tok[2].find('('), q = tok[2].find(')');
                int offset = stoi(tok[2].substr(0, p), nullptr, 0);
                int rs1 = reg_idx(tok[2].substr(p + 1, q - p - 1));
                code =
                    (offset << 20) | (rs1 << 15) | (3 << 12) | (rd << 7) | 0x03;
            } else if (inst == "sd") {
                if (tok.size() != 3)
                    throw runtime_error("sd 参数数量错误");
                int rs2 = reg_idx(tok[1]);
                auto p = tok[2].find('('), q = tok[2].find(')');
                int offset = stoi(tok[2].substr(0, p), nullptr, 0);
                int rs1 = reg_idx(tok[2].substr(p + 1, q - p - 1));
                uint32_t imm = offset & 0xFFF;
                code = ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) |
                       (3 << 12) | ((imm & 0x1F) << 7) | 0x23;
            } else if (inst == "add" || inst == "sub" || inst == "mul" ||
                       inst == "and" || inst == "or" || inst == "xor") {
                if (tok.size() != 4)
                    throw runtime_error("R型参数数量错误");
                int rd = reg_idx(tok[1]), rs1 = reg_idx(tok[2]),
                    rs2 = reg_idx(tok[3]);
                int funct3 = (inst == "and"   ? 7
                              : inst == "or"  ? 6
                              : inst == "xor" ? 4
                                              : 0);
                int funct7 = (inst == "sub"   ? 0x20
                              : inst == "mul" ? 0x01
                                              : 0x00);
                code = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) |
                       (funct3 << 12) | (rd << 7) | 0x33;
            } else if (inst == "xori") {
                if (tok.size() != 4)
                    throw runtime_error("xori 参数数量错误");
                int rd = reg_idx(tok[1]), rs1 = reg_idx(tok[2]);
                int imm = stoi(tok[3], nullptr, 0);
                code = (imm << 20) | (rs1 << 15) | (4 << 12) | (rd << 7) | 0x13;
            } else if (inst == "beq" || inst == "bge") {
                if (tok.size() != 4)
                    throw runtime_error("B型参数数量错误");
                int rs1 = reg_idx(tok[1]), rs2 = reg_idx(tok[2]);
                int offset = stoi(tok[3], nullptr, 0);
                int imm = offset & 0x1FFF;
                int imm12 = (imm >> 12) & 1, imm11 = (imm >> 11) & 1;
                int imm10_5 = (imm >> 5) & 0x3F, imm4_1 = (imm >> 1) & 0xF;
                int funct3 = (inst == "beq" ? 0 : 5);
                code = (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) |
                       (rs1 << 15) | (funct3 << 12) | (imm4_1 << 8) |
                       (imm11 << 7) | 0x63;
            } else if (inst == "jal") {
                if (tok.size() != 2)
                    throw runtime_error("jal 参数数量错误");

                int target;
                try {
                    target = stoi(tok[1], nullptr, 0);
                } catch (...) {
                    if (!label_addr.count(tok[1]))
                        throw runtime_error("未定义标签: " + tok[1]);
                    target = label_addr[tok[1]];
                }

                int offset = target - addr; // 关键：计算相对偏移
                if (offset % 2 != 0)
                    throw runtime_error("jal 偏移必须2字节对齐");

                int imm = offset & 0x1FFFFF; // 21位补码
                int imm20 = (imm >> 20) & 0x1;
                int imm10_1 = (imm >> 1) & 0x3FF;
                int imm11 = (imm >> 11) & 0x1;
                int imm19_12 = (imm >> 12) & 0xFF;

                int rd = 1; // x1 = return address
                code = (imm20 << 31) | (imm19_12 << 12) | (imm11 << 20) |
                       (imm10_1 << 21) | (rd << 7) | 0x6F;
            }

            else if (inst == "jr") {
                if (tok.size() != 2)
                    throw runtime_error("jr 参数数量错误");
                int offset = stoi(tok[1], nullptr, 0);
                code = (offset << 20) | (0 << 15) | (0 << 12) | (0 << 7) | 0x67;
            } else {
                throw runtime_error("未知指令: " + inst);
            }

            write_uint32_be(fout, code);
        } catch (exception& e) {
            cerr << "错误（地址 " << hex << addr << "）：" << e.what() << "\n";
            return 1;
        }
    }

    fout.close();
    cout << "汇编成功，已写入: " << output_file << "\n";
    return 0;
}