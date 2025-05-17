// #include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Helper to trim whitespace from both ends of string
static inline string trim(const string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Helper to split a string by whitespace (space or tab)
static vector<string> split(const string& s) {
    vector<string> tokens;
    istringstream iss(s);
    string tok;
    while (iss >> tok) {
        tokens.push_back(tok);
    }
    return tokens;
}

// Parse register name, expecting "xN" or "rN"
static int parseRegister(const string& s) {
    if (s.size() < 2)
        return -1;
    if (s[0] == 'x' || s[0] == 'r') {
        int reg = stoi(s.substr(1));
        return reg;
    }
    return -1;
}

// Convert string to integer, support decimal or 0x hex
static int64_t parseNumber(const string& s) {
    int base = 10;
    string str = s;
    if (str.size() > 1 && str[0] == '0' && str[1] == 'x') {
        base = 16;
        str = str.substr(2);
    }
    return stoll(str, nullptr, base);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: assembler <input_file> <output_base>" << endl;
        return 1;
    }
    string input_file = argv[1];
    string output_base = argv[2];
    ifstream infile(input_file);
    if (!infile) {
        cerr << "Error: cannot open input file " << input_file << endl;
        return 1;
    }
    // Read all lines of input
    vector<string> lines;
    string line;
    while (getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // First pass: record label addresses
    map<string, int64_t> label_addr; // 修改: 添加符号表用于存储标签地址
    int64_t PC = 0;
    for (auto& orig_line : lines) {
        string s = orig_line;
        // Remove comments (';' and everything after)
        size_t comment_pos = s.find(';');
        if (comment_pos != string::npos) {
            s = s.substr(0, comment_pos);
        }
        s = trim(s);
        if (s.empty())
            continue;
        // Check for label or address directive
        size_t colon_pos = s.find(':');
        if (colon_pos != string::npos) {
            string token = trim(s.substr(0, colon_pos));
            string rest = trim(s.substr(colon_pos + 1));
            if (token.size() > 0) {
                // Address directive or label
                if ((token.size() > 1 && token[0] == '0' && token[1] == 'x') ||
                    isdigit(token[0])) {
                    // Address directive
                    int64_t addr = parseNumber(token);
                    PC = addr; // 修改: 支持地址声明
                    // Continue to parse rest of line (if any) as instruction
                    if (!rest.empty()) {
                        // This rest is an instruction, so count it
                        PC += 4;
                    }
                } else {
                    // It's a label
                    label_addr[token] = PC; // 修改: 记录标签地址
                    // If there's code after label on same line
                    if (!rest.empty()) {
                        // treat 'rest' as instruction for first pass (increment
                        // PC)
                        PC += 4;
                    }
                }
            }
        } else {
            // No label, just an instruction
            PC += 4;
        }
    }

    // Prepare output files
    string bin_filename = output_base + ".bin";
    string cpp_filename = output_base + ".cpp";
    ofstream binfile(bin_filename, ios::binary);
    ofstream cppfile(cpp_filename);
    if (!binfile || !cppfile) {
        cerr << "Error: cannot open output files." << endl;
        return 1;
    }

    // Second pass: generate machine code
    PC = 0;
    for (auto& orig_line : lines) {
        string s = orig_line;
        // Remove comments
        size_t comment_pos = s.find(';');
        if (comment_pos != string::npos) {
            s = s.substr(0, comment_pos);
        }
        s = trim(s);
        if (s.empty())
            continue;
        // Check for label or address directive
        size_t colon_pos = s.find(':');
        if (colon_pos != string::npos) {
            string token = trim(s.substr(0, colon_pos));
            string rest = trim(s.substr(colon_pos + 1));
            if (token.size() > 0) {
                if ((token.size() > 1 && token[0] == '0' && token[1] == 'x') ||
                    isdigit(token[0])) {
                    // Address directive
                    int64_t addr = parseNumber(token);
                    PC = addr; // 修改: 支持地址声明
                    // Continue to parse rest of line as instruction
                    s = rest;
                } else {
                    // Label definition
                    label_addr[token]; // ensure label exists
                    s = rest;
                }
            }
        }
        s = trim(s);
        if (s.empty())
            continue;
        // Now s should be an instruction
        vector<string> tokens = split(s);
        string op = tokens[0];
        // Convert op to lowercase
        for (auto& c : op)
            c = tolower(c);

        uint32_t inst = 0;
        if (op == "add" || op == "sub" || op == "mul" || op == "div" ||
            op == "sll" || op == "srl" || op == "and" || op == "or" ||
            op == "xor") {
            // R-type
            // Syntax: add rd rs1 rs2
            if (tokens.size() < 4) {
                cerr << "Error: not enough operands for " << op << endl;
                return 1;
            }
            int rd = parseRegister(tokens[1]);
            int rs1 = parseRegister(tokens[2]);
            int rs2 = parseRegister(tokens[3]);
            if (rd < 0 || rs1 < 0 || rs2 < 0) {
                cerr << "Error: invalid register" << endl;
                return 1;
            }
            int funct3 = 0, funct7 = 0;
            uint32_t opcode = 0x33;
            if (op == "add") {
                funct3 = 0x0;
                funct7 = 0x00;
            } else if (op == "sub") {
                funct3 = 0x0;
                funct7 = 0x20;
            } else if (op == "mul") {
                funct3 = 0x0;
                funct7 = 0x01;
            } else if (op == "div") {
                funct3 = 0x4;
                funct7 = 0x01;
            } else if (op == "sll") {
                funct3 = 0x1;
                funct7 = 0x00;
            } else if (op == "srl") {
                funct3 = 0x5;
                funct7 = 0x00;
            } else if (op == "and") {
                funct3 = 0x7;
                funct7 = 0x00;
            } else if (op == "or") {
                funct3 = 0x6;
                funct7 = 0x00;
            } else if (op == "xor") {
                funct3 = 0x4;
                funct7 = 0x00;
            }
            inst = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
                   (rd << 7) | opcode;
        } else if (op == "addi") {
            // I-type: addi rd rs1 imm
            if (tokens.size() < 4) {
                cerr << "Error: not enough operands for addi" << endl;
                return 1;
            }
            int rd = parseRegister(tokens[1]);
            int rs1 = parseRegister(tokens[2]);
            int imm = parseNumber(tokens[3]);
            uint32_t opcode = 0x13;
            uint32_t funct3 = 0x0;
            uint32_t imm12 = imm & 0xFFF;
            inst = (imm12 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) |
                   opcode;
        } else if (op == "ld") {
            // I-type load: ld rd, imm(rs1)
            if (tokens.size() < 3) {
                cerr << "Error: not enough operands for ld" << endl;
                return 1;
            }
            int rd = parseRegister(tokens[1]);
            string mem = tokens[2];
            if (mem.back() == ',')
                mem.pop_back();
            size_t lpar = mem.find('(');
            size_t rpar = mem.find(')');
            if (lpar == string::npos || rpar == string::npos) {
                // maybe tokens[2]=imm, tokens[3]=reg
                if (tokens.size() < 4) {
                    cerr << "Error: invalid format for ld" << endl;
                    return 1;
                }
                int imm = parseNumber(tokens[2]);
                int rs1 = parseRegister(tokens[3]);
                uint32_t opcode = 0x03;
                uint32_t funct3 = 0x3;
                uint32_t imm12 = imm & 0xFFF;
                inst = (imm12 << 20) | (rs1 << 15) | (funct3 << 12) |
                       (rd << 7) | opcode;
            } else {
                string imm_str = mem.substr(0, lpar);
                string rs1_str = mem.substr(lpar + 1, rpar - lpar - 1);
                int imm = parseNumber(imm_str);
                int rs1 = parseRegister(rs1_str);
                uint32_t opcode = 0x03;
                uint32_t funct3 = 0x3; // for LD
                uint32_t imm12 = imm & 0xFFF;
                inst = (imm12 << 20) | (rs1 << 15) | (funct3 << 12) |
                       (rd << 7) | opcode;
            }
        } else if (op == "sd") {
            // S-type store: sd rs2, imm(rs1)
            if (tokens.size() < 3) {
                cerr << "Error: not enough operands for sd" << endl;
                return 1;
            }
            int rs2 = parseRegister(tokens[1]);
            string mem = tokens[2];
            if (mem.back() == ',')
                mem.pop_back();
            size_t lpar = mem.find('(');
            size_t rpar = mem.find(')');
            if (lpar == string::npos || rpar == string::npos) {
                if (tokens.size() < 4) {
                    cerr << "Error: invalid format for sd" << endl;
                    return 1;
                }
                int imm = parseNumber(tokens[2]);
                int rs1 = parseRegister(tokens[3]);
                uint32_t opcode = 0x23;
                uint32_t funct3 = 0x3;
                uint32_t imm12 = imm & 0xFFF;
                uint32_t imm11_5 = (imm12 >> 5) & 0x7F;
                uint32_t imm4_0 = imm12 & 0x1F;
                inst = (imm11_5 << 25) | (rs2 << 20) | (rs1 << 15) |
                       (funct3 << 12) | (imm4_0 << 7) | opcode;
            } else {
                string imm_str = mem.substr(0, lpar);
                string rs1_str = mem.substr(lpar + 1, rpar - lpar - 1);
                int imm = parseNumber(imm_str);
                int rs1 = parseRegister(rs1_str);
                uint32_t opcode = 0x23;
                uint32_t funct3 = 0x3;
                uint32_t imm12 = imm & 0xFFF;
                uint32_t imm11_5 = (imm12 >> 5) & 0x7F;
                uint32_t imm4_0 = imm12 & 0x1F;
                inst = (imm11_5 << 25) | (rs2 << 20) | (rs1 << 15) |
                       (funct3 << 12) | (imm4_0 << 7) | opcode;
            }
        } else if (op == "beq" || op == "blt" || op == "bge") {
            // B-type branch: beq rs1 rs2 offset/label
            if (tokens.size() < 4) {
                cerr << "Error: not enough operands for " << op << endl;
                return 1;
            }
            int rs1 = parseRegister(tokens[1]);
            int rs2 = parseRegister(tokens[2]);
            int64_t offset = 0;
            string target = tokens[3];
            if (target.back() == ',')
                target.pop_back();
            if (isalpha(target[0])) {
                if (label_addr.find(target) == label_addr.end()) {
                    cerr << "Error: undefined label " << target << endl;
                    return 1;
                }
                int64_t targetAddr = label_addr[target];
                offset = targetAddr - PC; // 修改: 计算相对偏移
            } else {
                offset = parseNumber(target) - PC;
            }
            int64_t imm = offset >> 1;
            uint32_t opcode = 0x63;
            uint32_t funct3 = 0;
            if (op == "beq")
                funct3 = 0x0;
            else if (op == "blt")
                funct3 = 0x4;
            else if (op == "bge")
                funct3 = 0x5;
            uint32_t imm12 = (imm >> 11) & 0x1;
            uint32_t imm10_5 = (imm >> 5) & 0x3F;
            uint32_t imm4_1 = (imm >> 1) & 0xF;
            uint32_t imm11 = (imm >> 10) & 0x1;
            inst = (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) | (rs1 << 15) |
                   (funct3 << 12) | (imm4_1 << 8) | (imm11 << 7) | opcode;
        } else if (op == "jal") {
            // J-type: jal rd offset/label
            if (tokens.size() < 2) {
                cerr << "Error: not enough operands for jal" << endl;
                return 1;
            }
            int rd;
            string target;
            if (tokens.size() == 2) {
                rd = 1; // 默认 x1
                target = tokens[1];
            } else {
                rd = parseRegister(tokens[1]);
                target = tokens[2];
            }
            if (target.back() == ',')
                target.pop_back();
            int64_t offset = 0;
            if (isalpha(target[0])) {
                if (label_addr.find(target) == label_addr.end()) {
                    cerr << "Error: undefined label " << target << endl;
                    return 1;
                }
                offset = label_addr[target] - PC; // 修改: 计算相对偏移
            } else {
                offset = parseNumber(target) - PC;
            }
            int64_t imm = offset >> 1;
            uint32_t opcode = 0x6F;
            uint32_t imm20 = (imm >> 19) & 0x1;
            uint32_t imm10_1 = imm & 0x3FF;
            uint32_t imm11 = (imm >> 10) & 0x1;
            uint32_t imm19_12 = (imm >> 11) & 0xFF;
            inst = (imm20 << 31) | (imm19_12 << 12) | (imm11 << 20) |
                   (imm10_1 << 21) | (rd << 7) | opcode;
        } else if (op == "jalr" || op == "jr") {
            // I-type: jalr rd, imm(rs1) or jr label (treated as jal x0)
            int rd = 0, rs1 = 0;
            int64_t offset = 0;
            if (op == "jr") {
                // treat jr as jal x0, label
                rd = 0;
                string target = tokens[1];
                if (target.back() == ',')
                    target.pop_back();
                if (isalpha(target[0])) {
                    if (label_addr.find(target) == label_addr.end()) {
                        cerr << "Error: undefined label " << target << endl;
                        return 1;
                    }
                    offset = label_addr[target] - PC; // 修改: 计算相对偏移
                } else {
                    offset = parseNumber(target) - PC;
                }
            } else {
                // jalr
                if (tokens.size() < 3) {
                    cerr << "Error: not enough operands for jalr" << endl;
                    return 1;
                }
                string rd_str = tokens[1];
                if (rd_str.back() == ',')
                    rd_str.pop_back();
                rd = parseRegister(rd_str);
                string mem = tokens[2];
                if (mem.back() == ',')
                    mem.pop_back();
                size_t lpar = mem.find('(');
                size_t rpar = mem.find(')');
                if (lpar == string::npos || rpar == string::npos) {
                    cerr << "Error: invalid format for jalr" << endl;
                    return 1;
                }
                string imm_str = mem.substr(0, lpar);
                string rs1_str = mem.substr(lpar + 1, rpar - lpar - 1);
                offset = parseNumber(imm_str);
                rs1 = parseRegister(rs1_str);
            }
            uint32_t opcode = 0x67;
            uint32_t funct3 = 0x0;
            uint32_t imm12 = offset & 0xFFF;
            inst = (imm12 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) |
                   opcode;
        } else if (op == "lui") {
            // U-type: lui rd imm
            if (tokens.size() < 3) {
                cerr << "Error: not enough operands for lui" << endl;
                return 1;
            }
            int rd = parseRegister(tokens[1]);
            int imm = parseNumber(tokens[2]);
            uint32_t opcode = 0x37;
            uint32_t imm20 = (imm & 0xFFFFF) << 12;
            inst = (imm20) | (rd << 7) | opcode;
        } else {
            cerr << "Error: unknown instruction " << op << endl;
            return 1;
        }

        // Write instruction to binary (big endian) and .cpp file
        for (int i = 3; i >= 0; i--) {
            uint8_t byte = (inst >> (i * 8)) & 0xFF;
            binfile.put(byte);
            cppfile << "write_byte(0x" << hex << PC << ", 0x" << (int)byte
                    << dec << ");" << endl;
            PC++;
        }
    }

    binfile.close();
    cppfile.close();
    return 0;
}