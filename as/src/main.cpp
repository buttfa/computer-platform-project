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

vector<string> tokenize(const string& line) {
    vector<string> res;
    stringstream ss(line);
    string word;
    while (ss >> word)
        res.push_back(word);
    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: assembler <output_file> [start_addr, 默认0x1000]\n";
        return 1;
    }

    const char* output_file = argv[1];
    uint64_t start_addr = (argc >= 3) ? strtoull(argv[2], nullptr, 0) : 0x1000;

    unordered_map<string, int> label_addr;
    vector<pair<int, vector<string>>> program;
    unordered_set<int> used;
    string line;
    int curr_addr = 0;
    bool have_prev = false;
    int line_no = 0;

    // 第一遍：记录地址和标签
    while (getline(cin, line)) {
        ++line_no;
        size_t comment = line.find(';');
        if (comment != string::npos)
            line = line.substr(0, comment);
        line.erase(0, line.find_first_not_of(" \t\r\n"));
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

        if (tok[0].substr(0, 2) == "0x") {
            curr_addr = stoi(tok[0], nullptr, 0);
            tok.erase(tok.begin());
        } else if (have_prev) {
            curr_addr += 4;
        } else {
            curr_addr = 0;
        }

        if (used.count(curr_addr)) {
            cerr << "错误：地址重复：" << hex << curr_addr << '\n';
            return 1;
        }

        used.insert(curr_addr);
        have_prev = true;
        program.emplace_back(curr_addr, tok);
    }

    ofstream fout(output_file, ios::binary);
    if (!fout) {
        cerr << "无法打开输出文件: " << output_file << '\n';
        return 1;
    }

    // write_uint32_be(fout, 0);
    // write_uint64_be(fout, start_addr);

    for (auto& [addr, tok] : program) {
        string inst = tok[0];
        uint32_t code = 0;

    retry:
        try {
            if (inst == "not") {
                if (tok.size() != 3)
                    throw runtime_error("not 格式错误");
                tok = {"xori", tok[1], tok[2], "-1"};
                inst = "xori";
            }

            if (inst == "jalri") {
                if (tok.size() != 2)
                    throw runtime_error("jalri 格式错误");
                string fake = "jalr";
                string imm_reg = tok[1] + "(x0)";
                tok = {fake, "x1", imm_reg};
                inst = "jalr";
                goto retry;
            }

            if (inst == "ret") {
                code = (0 << 20) | (1 << 15) | (0 << 12) | (0 << 7) | 0x67;
                write_uint32_be(fout, code);
                continue;
            }

            if (inst == "blt")
                throw runtime_error("不支持 blt 指令");

            if (inst == "lui") {
                int rd = reg_idx(tok[1]);
                int imm = stoi(tok[2], nullptr, 0);
                code = (imm << 12) | (rd << 7) | 0x37;
            } else if (inst == "ld") {
                int rd = reg_idx(tok[1]);
                size_t p = tok[2].find('('), q = tok[2].find(')');
                int offset = stoi(tok[2].substr(0, p), nullptr, 0);
                int rs1 = reg_idx(tok[2].substr(p + 1, q - p - 1));
                code =
                    (offset << 20) | (rs1 << 15) | (3 << 12) | (rd << 7) | 0x03;
            } else if (inst == "sd") {
                int rs2 = reg_idx(tok[1]);
                size_t p = tok[2].find('('), q = tok[2].find(')');
                int offset = stoi(tok[2].substr(0, p), nullptr, 0);
                int rs1 = reg_idx(tok[2].substr(p + 1, q - p - 1));
                code = ((offset >> 5) << 25) | (rs2 << 20) | (rs1 << 15) |
                       (3 << 12) | ((offset & 0x1F) << 7) | 0x23;
            } else if (inst == "add" || inst == "sub" || inst == "mul" ||
                       inst == "and" || inst == "or" || inst == "xor" ||
                       inst == "div" || inst == "sll" || inst == "srl") {
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
                } else if (inst == "div") {
                    funct3 = 4;
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
                } else if (inst == "sll") {
                    funct3 = 1;
                    funct7 = 0x00;
                } else if (inst == "srl") {
                    funct3 = 5;
                    funct7 = 0x00;
                }

                code = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) |
                       (funct3 << 12) | (rd << 7) | 0x33;
            } else if (inst == "xori" || inst == "addi") {
                int rd = reg_idx(tok[1]), rs1 = reg_idx(tok[2]);
                int imm = stoi(tok[3], nullptr, 0);
                int funct3 = (inst == "xori" ? 4 : 0);
                code = (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) |
                       0x13;
            } else if (inst == "beq" || inst == "bge") {
                int rs1 = reg_idx(tok[1]), rs2 = reg_idx(tok[2]);
                int target;
                try {
                    target = stoi(tok[3], nullptr, 0);
                } catch (...) {
                    if (!label_addr.count(tok[3]))
                        throw runtime_error("未定义标签: " + tok[3]);
                    target = label_addr[tok[3]];
                }
                int offset = target - addr;
                if (offset % 2 != 0)
                    throw runtime_error("beq 偏移未对齐");

                int imm12 = (offset >> 12) & 1;
                int imm10_5 = (offset >> 5) & 0x3F;
                int imm4_1 = (offset >> 1) & 0xF;
                int imm11 = (offset >> 11) & 1;
                int funct3 = (inst == "beq") ? 0 : 5;

                code = (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) |
                       (rs1 << 15) | (funct3 << 12) | (imm4_1 << 8) |
                       (imm11 << 7) | 0x63;
            } else if (inst == "jal") {
                int target;
                try {
                    target = stoi(tok[1], nullptr, 0);
                } catch (...) {
                    if (!label_addr.count(tok[1]))
                        throw runtime_error("未定义标签: " + tok[1]);
                    target = label_addr[tok[1]];
                }
                int offset = target - addr;
                if (offset % 2 != 0)
                    throw runtime_error("jal 偏移未对齐");

                int imm20 = (offset >> 20) & 1;
                int imm10_1 = (offset >> 1) & 0x3FF;
                int imm11 = (offset >> 11) & 1;
                int imm19_12 = (offset >> 12) & 0xFF;
                int rd = 1;

                code = (imm20 << 31) | (imm19_12 << 12) | (imm11 << 20) |
                       (imm10_1 << 21) | (rd << 7) | 0x6F;
            } else if (inst == "jr") {
                int target;
                try {
                    target = stoi(tok[1], nullptr, 0);
                } catch (...) {
                    if (!label_addr.count(tok[1]))
                        throw runtime_error("未定义标签: " + tok[1]);
                    target = label_addr[tok[1]];
                }
                int offset = target - addr;
                code = (offset << 20) | (0 << 15) | (0 << 12) | (0 << 7) | 0x67;
            } else if (inst == "jalr") {
                if (tok.size() != 3)
                    throw runtime_error("jalr 格式错误");
                int rd = reg_idx(tok[1]);
                size_t p = tok[2].find('('), q = tok[2].find(')');
                if (p == string::npos || q == string::npos)
                    throw runtime_error("jalr 偏移语法错误，应为 imm(rs1)");
                int imm = stoi(tok[2].substr(0, p), nullptr, 0);
                int rs1 = reg_idx(tok[2].substr(p + 1, q - p - 1));
                code = (imm << 20) | (rs1 << 15) | (0 << 12) | (rd << 7) | 0x67;
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
    cout << "汇编成功，输出文件: " << output_file << "\n";
    return 0;
}