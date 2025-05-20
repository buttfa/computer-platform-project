#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Trim whitespace from both ends of a string
static inline string trim(const string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Parse a register name like x0..x31
int parseRegister(const string& reg) {
    if (reg.size() > 1 && reg[0] == 'x') {
        try {
            int num = stoi(reg.substr(1));
            if (num >= 0 && num <= 31)
                return num;
        } catch (...) {
        }
    }
    return -1; // invalid
}

// Write a 32-bit word to output file in little-endian
void writeWord(ofstream& out, uint32_t word) {
    out.put((char)(word & 0xFF));
    out.put((char)((word >> 8) & 0xFF));
    out.put((char)((word >> 16) & 0xFF));
    out.put((char)((word >> 24) & 0xFF));
}

// Split string by delimiter, trimming spaces
vector<string> split(const string& s, char delim) {
    vector<string> elems;
    string item;
    stringstream ss(s);
    while (getline(ss, item, delim)) {
        elems.push_back(trim(item));
    }
    return elems;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }
    string input_file = argv[1];
    string output_file = argv[2];
    ifstream fin(input_file);
    if (!fin) {
        cerr << "Error: cannot open input file " << input_file << endl;
        return 1;
    }
    // Read all lines
    vector<string> lines;
    string line;
    while (getline(fin, line)) {
        // Remove comments starting with '#'
        size_t pos = line.find('#');
        if (pos != string::npos) {
            line = line.substr(0, pos);
        }
        line = trim(line);
        lines.push_back(line);
    }
    fin.close();

    // First pass: collect labels
    unordered_map<string, int> labels;
    int pc = 0;
    for (auto& ln : lines) {
        if (ln.empty())
            continue;
        // Check for label
        size_t colon = ln.find(':');
        if (colon != string::npos) {
            string label = trim(ln.substr(0, colon));
            if (label.empty()) {
                cerr << "Error: empty label definition" << endl;
                return 1;
            }
            if (labels.count(label)) {
                cerr << "Error: duplicate label " << label << endl;
                return 1;
            }
            labels[label] = pc;
            // If there's code after label:
            string rest = trim(ln.substr(colon + 1));
            if (rest.empty())
                continue;
            ln = rest;
        }
        // If not empty after label removal, it's an instruction line
        if (!ln.empty()) {
            pc += 4;
        }
    }

    // Second pass: assemble
    pc = 0;
    vector<uint32_t> machine;
    for (auto& orig_line : lines) {
        string ln = orig_line;
        if (ln.empty())
            continue;
        // Handle label in line if any
        size_t colon = ln.find(':');
        if (colon != string::npos) {
            ln = trim(ln.substr(colon + 1));
            if (ln.empty())
                continue;
        }
        if (ln.empty())
            continue;
        stringstream ss(ln);
        string inst;
        ss >> inst;
        if (inst == "ret") {
            // Pseudo: jalr x0, 0(x1)
            uint32_t code = (0 << 20) | (1 << 15) | (0 << 12) | (0 << 7) | 0x67;
            machine.push_back(code);
            pc += 4;
        } else if (inst == "not") {
            // Pseudo: xori rd, rs, -1
            string rd_str, rs_str;
            if (!(ss >> rd_str)) {
                cerr << "Error: missing destination register for 'not'" << endl;
                return 1;
            }
            if (rd_str.back() == ',')
                rd_str.pop_back();
            if (!(ss >> rs_str)) {
                cerr << "Error: missing source register for 'not'" << endl;
                return 1;
            }
            if (rs_str.back() == ',')
                rs_str.pop_back();
            int rd = parseRegister(rd_str);
            int rs = parseRegister(rs_str);
            if (rd < 0 || rs < 0) {
                cerr << "Error: invalid register in 'not'" << endl;
                return 1;
            }
            int imm = (-1) & 0xFFF;
            uint32_t code = ((imm & 0xFFF) << 20) | (rs << 15) | (0b100 << 12) |
                            (rd << 7) | 0x13;
            machine.push_back(code);
            pc += 4;
        } else if (inst == "blt") {
            cerr << "Error: unknown instruction '" << inst << "'" << endl;
            return 1;
        } else {
            if (inst == "add" || inst == "sub" || inst == "and" ||
                inst == "or" || inst == "xor" || inst == "sll" ||
                inst == "srl" || inst == "sra" || inst == "slt" ||
                inst == "sltu") {
                string rd_str, rs1_str, rs2_str;
                if (!(ss >> rd_str)) {
                    cerr << "Error: missing rd for " << inst << endl;
                    return 1;
                }
                if (rd_str.back() == ',')
                    rd_str.pop_back();
                if (!(ss >> rs1_str)) {
                    cerr << "Error: missing rs1 for " << inst << endl;
                    return 1;
                }
                if (rs1_str.back() == ',')
                    rs1_str.pop_back();
                if (!(ss >> rs2_str)) {
                    cerr << "Error: missing rs2 for " << inst << endl;
                    return 1;
                }
                if (rs2_str.back() == ',')
                    rs2_str.pop_back();
                int rd = parseRegister(rd_str);
                int rs1 = parseRegister(rs1_str);
                int rs2 = parseRegister(rs2_str);
                if (rd < 0 || rs1 < 0 || rs2 < 0) {
                    cerr << "Error: invalid register in " << inst << endl;
                    return 1;
                }
                int funct7 = 0, funct3 = 0;
                uint32_t opcode = 0;
                if (inst == "add") {
                    funct7 = 0;
                    funct3 = 0;
                    opcode = 0x33;
                }
                if (inst == "sub") {
                    funct7 = 0x20;
                    funct3 = 0;
                    opcode = 0x33;
                }
                if (inst == "and") {
                    funct7 = 0;
                    funct3 = 7;
                    opcode = 0x33;
                }
                if (inst == "or") {
                    funct7 = 0;
                    funct3 = 6;
                    opcode = 0x33;
                }
                if (inst == "xor") {
                    funct7 = 0;
                    funct3 = 4;
                    opcode = 0x33;
                }
                if (inst == "sll") {
                    funct7 = 0;
                    funct3 = 1;
                    opcode = 0x33;
                }
                if (inst == "srl") {
                    funct7 = 0;
                    funct3 = 5;
                    opcode = 0x33;
                }
                if (inst == "sra") {
                    funct7 = 0x20;
                    funct3 = 5;
                    opcode = 0x33;
                }
                if (inst == "slt") {
                    funct7 = 0;
                    funct3 = 2;
                    opcode = 0x33;
                }
                if (inst == "sltu") {
                    funct7 = 0;
                    funct3 = 3;
                    opcode = 0x33;
                }
                uint32_t code = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) |
                                (funct3 << 12) | (rd << 7) | opcode;
                machine.push_back(code);
                pc += 4;
            } else if (inst == "addi" || inst == "andi" || inst == "ori" ||
                       inst == "xori" || inst == "slti" || inst == "sltiu" ||
                       inst == "lb" || inst == "lh" || inst == "lw" ||
                       inst == "jalr") {
                string rd_str, rs1_str, imm_str;
                if (!(ss >> rd_str)) {
                    cerr << "Error: missing rd for " << inst << endl;
                    return 1;
                }
                if (rd_str.back() == ',')
                    rd_str.pop_back();
                if (!(ss >> rs1_str)) {
                    cerr << "Error: missing operand for " << inst << endl;
                    return 1;
                }
                int rd = parseRegister(rd_str);
                if (rd < 0) {
                    cerr << "Error: invalid register in " << inst << endl;
                    return 1;
                }
                int rs1 = -1;
                int imm = 0;
                size_t p = rs1_str.find('(');
                if (p != string::npos) {
                    // format imm(rs1)
                    string imm_part = rs1_str.substr(0, p);
                    string rs1_reg = rs1_str.substr(p + 1);
                    if (rs1_reg.back() == ')')
                        rs1_reg.pop_back();
                    rs1 = parseRegister(rs1_reg);
                    if (rs1 < 0) {
                        cerr << "Error: invalid register in " << inst << endl;
                        return 1;
                    }
                    try {
                        imm = stoi(imm_part);
                    } catch (...) {
                        cerr << "Error: invalid immediate " << imm_part << endl;
                        return 1;
                    }
                } else {
                    // format rd, rs1, imm
                    if (rs1_str.back() == ',')
                        rs1_str.pop_back();
                    rs1 = parseRegister(rs1_str);
                    if (rs1 < 0) {
                        cerr << "Error: invalid register in " << inst << endl;
                        return 1;
                    }
                    if (!(ss >> imm_str)) {
                        cerr << "Error: missing immediate in " << inst << endl;
                        return 1;
                    }
                    try {
                        imm = stoi(imm_str);
                    } catch (...) {
                        cerr << "Error: invalid immediate " << imm_str << endl;
                        return 1;
                    }
                }
                int funct3 = 0;
                uint32_t opcode = 0x13;
                if (inst == "addi")
                    funct3 = 0;
                if (inst == "andi")
                    funct3 = 7;
                if (inst == "ori")
                    funct3 = 6;
                if (inst == "xori")
                    funct3 = 4;
                if (inst == "slti")
                    funct3 = 2;
                if (inst == "sltiu")
                    funct3 = 3;
                if (inst == "lb") {
                    funct3 = 0;
                    opcode = 0x03;
                }
                if (inst == "lh") {
                    funct3 = 1;
                    opcode = 0x03;
                }
                if (inst == "lw") {
                    funct3 = 2;
                    opcode = 0x03;
                }
                if (inst == "jalr") {
                    funct3 = 0;
                    opcode = 0x67;
                }
                if (imm < -2048 || imm > 2047) {
                    cerr << "Error: immediate out of range in " << inst << endl;
                    return 1;
                }
                uint32_t imm12 = (uint32_t)(imm & 0xFFF);
                uint32_t code = (imm12 << 20) | (rs1 << 15) | (funct3 << 12) |
                                (rd << 7) | opcode;
                machine.push_back(code);
                pc += 4;
            } else if (inst == "sw" || inst == "sb" || inst == "sh") {
                string rs2_str, offset_reg;
                if (!(ss >> rs2_str)) {
                    cerr << "Error: missing rs2 for " << inst << endl;
                    return 1;
                }
                if (rs2_str.back() == ',')
                    rs2_str.pop_back();
                if (!(ss >> offset_reg)) {
                    cerr << "Error: missing operand for " << inst << endl;
                    return 1;
                }
                int rs2 = parseRegister(rs2_str);
                if (rs2 < 0) {
                    cerr << "Error: invalid register in " << inst << endl;
                    return 1;
                }
                size_t p = offset_reg.find('(');
                if (p == string::npos) {
                    cerr << "Error: missing '(' in " << inst << endl;
                    return 1;
                }
                string imm_str = offset_reg.substr(0, p);
                string rs1_reg = offset_reg.substr(p + 1);
                if (rs1_reg.back() == ')')
                    rs1_reg.pop_back();
                int rs1 = parseRegister(rs1_reg);
                if (rs1 < 0) {
                    cerr << "Error: invalid register in " << inst << endl;
                    return 1;
                }
                int imm = 0;
                try {
                    imm = stoi(imm_str);
                } catch (...) {
                    cerr << "Error: invalid immediate " << imm_str << endl;
                    return 1;
                }
                if (imm < -2048 || imm > 2047) {
                    cerr << "Error: immediate out of range in " << inst << endl;
                    return 1;
                }
                uint32_t imm12 = (uint32_t)(imm & 0xFFF);
                uint32_t imm11_5 = (imm12 >> 5) & 0x7F;
                uint32_t imm4_0 = imm12 & 0x1F;
                int funct3 = 0;
                if (inst == "sb")
                    funct3 = 0;
                if (inst == "sh")
                    funct3 = 1;
                if (inst == "sw")
                    funct3 = 2;
                uint32_t opcode = 0x23;
                uint32_t code = (imm11_5 << 25) | (rs2 << 20) | (rs1 << 15) |
                                (funct3 << 12) | (imm4_0 << 7) | opcode;
                machine.push_back(code);
                pc += 4;
            } else if (inst == "beq" || inst == "bne" || inst == "bge" ||
                       inst == "bgeu") {
                string rs1_str, rs2_str, label;
                if (!(ss >> rs1_str)) {
                    cerr << "Error: missing rs1 for " << inst << endl;
                    return 1;
                }
                if (rs1_str.back() == ',')
                    rs1_str.pop_back();
                if (!(ss >> rs2_str)) {
                    cerr << "Error: missing rs2 for " << inst << endl;
                    return 1;
                }
                if (rs2_str.back() == ',')
                    rs2_str.pop_back();
                if (!(ss >> label)) {
                    cerr << "Error: missing label for " << inst << endl;
                    return 1;
                }
                int rs1 = parseRegister(rs1_str);
                int rs2 = parseRegister(rs2_str);
                if (rs1 < 0 || rs2 < 0) {
                    cerr << "Error: invalid register in " << inst << endl;
                    return 1;
                }
                if (!labels.count(label)) {
                    cerr << "Error: undefined label " << label << endl;
                    return 1;
                }
                int target = labels[label];
                int offset = target - pc;
                if (offset % 2 != 0) {
                    cerr << "Error: branch target not aligned" << endl;
                    return 1;
                }
                int imm = offset;
                int imm12 = (imm >> 12) & 0x1;
                int imm10_5 = (imm >> 5) & 0x3F;
                int imm4_1 = (imm >> 1) & 0xF;
                int imm11 = (imm >> 11) & 0x1;
                int funct3 = 0;
                if (inst == "beq")
                    funct3 = 0;
                if (inst == "bne")
                    funct3 = 1;
                if (inst == "bge")
                    funct3 = 5;
                if (inst == "bgeu")
                    funct3 = 7;
                uint32_t opcode = 0x63;
                uint32_t code = (imm12 << 31) | (imm11 << 7) | (imm10_5 << 25) |
                                (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
                                (imm4_1 << 8) | opcode;
                machine.push_back(code);
                pc += 4;
            } else if (inst == "lui" || inst == "auipc") {
                string rd_str, imm_str;
                if (!(ss >> rd_str)) {
                    cerr << "Error: missing rd for " << inst << endl;
                    return 1;
                }
                if (rd_str.back() == ',')
                    rd_str.pop_back();
                if (!(ss >> imm_str)) {
                    cerr << "Error: missing immediate for " << inst << endl;
                    return 1;
                }
                int rd = parseRegister(rd_str);
                if (rd < 0) {
                    cerr << "Error: invalid register in " << inst << endl;
                    return 1;
                }
                int imm = 0;
                try {
                    imm = stoi(imm_str);
                } catch (...) {
                    cerr << "Error: invalid immediate " << imm_str << endl;
                    return 1;
                }
                uint32_t imm20 = (uint32_t)(imm & 0xFFFFF);
                uint32_t opcode = (inst == "lui" ? 0x37 : 0x17);
                uint32_t code = (imm20 << 12) | (rd << 7) | opcode;
                machine.push_back(code);
                pc += 4;
            } else if (inst == "jal") {
                string rd_str, label;
                if (!(ss >> rd_str)) {
                    cerr << "Error: missing rd for jal" << endl;
                    return 1;
                }
                if (rd_str.back() == ',')
                    rd_str.pop_back();
                if (!(ss >> label)) {
                    cerr << "Error: missing label for jal" << endl;
                    return 1;
                }
                int rd = parseRegister(rd_str);
                if (rd < 0) {
                    cerr << "Error: invalid register in jal" << endl;
                    return 1;
                }
                if (!labels.count(label)) {
                    cerr << "Error: undefined label " << label << endl;
                    return 1;
                }
                int target = labels[label];
                int offset = target - pc;
                int imm = offset;
                int imm20 = (imm >> 20) & 0x1;
                int imm10_1 = (imm >> 1) & 0x3FF;
                int imm11 = (imm >> 11) & 0x1;
                int imm19_12 = (imm >> 12) & 0xFF;
                uint32_t code = (imm20 << 31) | (imm19_12 << 12) |
                                (imm11 << 20) | (imm10_1 << 21) | (rd << 7) |
                                0x6F;
                machine.push_back(code);
                pc += 4;
            } else {
                cerr << "Error: unknown instruction '" << inst << "'" << endl;
                return 1;
            }
        }
    }

    // Write machine code to output binary file
    ofstream fout(output_file, ios::binary);
    if (!fout) {
        cerr << "Error: cannot open output file " << output_file << endl;
        return 1;
    }
    for (uint32_t word : machine) {
        writeWord(fout, word);
    }
    fout.close();
    return 0;
}