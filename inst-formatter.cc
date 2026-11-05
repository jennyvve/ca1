/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    inst-formatter.cc - RISC-V instruction printer (disassembler)
 *
 * Copyright (C) 2016,2018  Leiden University, The Netherlands.
 */

#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "inst-decoder.h"

std::string conv_reg_str(uint64_t reg) {
    // Array of register names, keep in order, index is mapped to
    // string.
    const std::string m_reg_text[] = {
        "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",  "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",
    };

    if (reg < 0 || reg > 31) {
        return "";
    }
    return m_reg_text[reg];
}

std::map<instruction_opcode_e, std::string> map_instr_str_uj_type = {
    {INSTRUCTION_OPCODE_JAL, "jal"},
    {INSTRUCTION_OPCODE_AUIPC, "auipc"},
};

std::map<instruction_opcode_e, std::string> map_instr_str_u_type = {};

std::map<std::pair<instruction_opcode_e, instruction_funct3_e>, std::string> map_instr_str_ibs_type = {
    {{INSTRUCTION_OPCODE_JALR, INSTRUCTION_FUNCT3_JALR}, "jalr"},
    {{INSTRUCTION_OPCODE_BRANCH, INSTRUCTION_FUNCT3_BRANCH_BEQ}, "beq"},
    {{INSTRUCTION_OPCODE_BRANCH, INSTRUCTION_FUNCT3_BRANCH_BNE}, "bne"},
    {{INSTRUCTION_OPCODE_BRANCH, INSTRUCTION_FUNCT3_BRANCH_BLT}, "blt"},
    {{INSTRUCTION_OPCODE_BRANCH, INSTRUCTION_FUNCT3_BRANCH_BGE}, "bge"},
    {{INSTRUCTION_OPCODE_BRANCH, INSTRUCTION_FUNCT3_BRANCH_BLTU}, "bltu"},
    {{INSTRUCTION_OPCODE_BRANCH, INSTRUCTION_FUNCT3_BRANCH_BGEU}, "bgeu"},
    {{INSTRUCTION_OPCODE_LUI, INSTRUCTION_FUNCT3_NONE}, "lui"},
    {{INSTRUCTION_OPCODE_LOAD, INSTRUCTION_FUNCT3_LOAD_LB}, "lb"},
    {{INSTRUCTION_OPCODE_LOAD, INSTRUCTION_FUNCT3_LOAD_LH}, "lh"},
    {{INSTRUCTION_OPCODE_LOAD, INSTRUCTION_FUNCT3_LOAD_LW}, "lw"},
    {{INSTRUCTION_OPCODE_LOAD, INSTRUCTION_FUNCT3_LOAD_LD}, "ld"},
    {{INSTRUCTION_OPCODE_LOAD, INSTRUCTION_FUNCT3_LOAD_LBU}, "lbu"},
    {{INSTRUCTION_OPCODE_LOAD, INSTRUCTION_FUNCT3_LOAD_LHU}, "lhu"},
    {{INSTRUCTION_OPCODE_STORE, INSTRUCTION_FUNCT3_STORE_SB}, "sb"},
    {{INSTRUCTION_OPCODE_STORE, INSTRUCTION_FUNCT3_STORE_SH}, "sh"},
    {{INSTRUCTION_OPCODE_STORE, INSTRUCTION_FUNCT3_STORE_SW}, "sw"},
    {{INSTRUCTION_OPCODE_STORE, INSTRUCTION_FUNCT3_STORE_SD}, "sd"},
    {{INSTRUCTION_OPCODE_OP_IMM_32, INSTRUCTION_FUNCT3_OP_IM_ADDI}, "addiw"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_ADDI}, "addi"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_ANDI}, "andi"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_SLLI}, "slli"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_SRLI_SRAI}, "srli"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_SRLI_SRAI}, "srai"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_SLTI}, "slti"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_SLTIU}, "sltiu"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_XORI}, "xori"},
    {{INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_FUNCT3_OP_IM_ORI}, "ori"},
    {{INSTRUCTION_OPCODE_JAL, INSTRUCTION_FUNCT3_NONE}, "jal"},

};

std::map<std::pair<instruction_opcode_e, std::pair<instruction_funct3_e, instruction_funct7_e>>, std::string>
    map_instr_str_r_type = {
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_ADD_SUB, INSTRUCTION_FUNCT7_OP_ADD}}, "add"},
        {{INSTRUCTION_OPCODE_OP_32, {INSTRUCTION_FUNCT3_OP_ADD_SUB, INSTRUCTION_FUNCT7_OP_SUB}}, "subw"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_ADD_SUB, INSTRUCTION_FUNCT7_OP_SUB}}, "sub"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_SLL, INSTRUCTION_FUNCT7_NONE}}, "sll"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_SLT, INSTRUCTION_FUNCT7_NONE}}, "slt"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_SLTU, INSTRUCTION_FUNCT7_NONE}}, "sltu"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_XOR, INSTRUCTION_FUNCT7_NONE}}, "xor"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_SRL_SRA, INSTRUCTION_FUNCT7_OP_ADD}}, "srl"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_SRL_SRA, INSTRUCTION_FUNCT7_OP_SUB}}, "sra"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_OR, INSTRUCTION_FUNCT7_NONE}}, "or"},
        {{INSTRUCTION_OPCODE_OP, {INSTRUCTION_FUNCT3_OP_AND, INSTRUCTION_FUNCT7_NONE}}, "and"},
};

std::ostream &operator<<(std::ostream &os, const InstructionDecoder &decoder) {
    instruction_t instr = decoder.get_instr();
    std::string instr_str;

    switch (instr.type) {
        case INSTRUCTION_TYPE_R:
            instr_str = map_instr_str_r_type[{instr.opcode, {instr.funct3, instr.funct7}}];
            os << instr_str << " " << conv_reg_str(instr.rd) << ", " << conv_reg_str(instr.rs1) << ", "
               << conv_reg_str(instr.rs2);
            break;
        case INSTRUCTION_TYPE_I:
            instr_str = map_instr_str_ibs_type[{instr.opcode, instr.funct3}];
            if (instr.opcode == INSTRUCTION_OPCODE_LOAD || instr.opcode == INSTRUCTION_OPCODE_JALR) {
                os << instr_str << " " << conv_reg_str(instr.rd) << ", " << instr.imm << "(" << conv_reg_str(instr.rs1)
                   << ")";
            } else {
                os << instr_str << " " << conv_reg_str(instr.rd) << ", " << conv_reg_str(instr.rs1) << ", "
                   << instr.imm;
            }
            break;
        case INSTRUCTION_TYPE_S:
            instr_str = map_instr_str_ibs_type[{instr.opcode, instr.funct3}];
            os << instr_str << " " << conv_reg_str(instr.rs2) << ", " << instr.imm << "(" << conv_reg_str(instr.rs1)
               << ")";
            break;
        case INSTRUCTION_TYPE_B:
            instr_str = map_instr_str_ibs_type[{instr.opcode, instr.funct3}];
            os << instr_str << " " << conv_reg_str(instr.rs1) << ", " << conv_reg_str(instr.rs2) << ", " << instr.imm;
            break;
        case INSTRUCTION_TYPE_U:
            instr_str = map_instr_str_uj_type[instr.opcode];
            os << instr_str << " " << conv_reg_str(instr.rd) << ", " << (instr.imm >> 12);
            break;
        case INSTRUCTION_TYPE_J:
            instr_str = map_instr_str_uj_type[instr.opcode];
            os << instr_str << " " << conv_reg_str(instr.rd) << ", " << instr.imm;
            break;
        case INSTRUCTION_TYPE_CI:
            os << instr_str << " " << conv_reg_str(instr.rd) << ", " << instr.imm;
            break;
        case INSTRUCTION_TYPE_CR:
            os << instr_str << " " << conv_reg_str(instr.rd) << ", " << conv_reg_str(instr.rs2);
            break;
        case INSTRUCTION_TYPE_CSS:
            os << instr_str << " " << conv_reg_str(instr.rs2) << ", " << instr.imm << "(sp)";
            break;
        case INSTRUCTION_TYPE_CIW:
            os << instr_str << " " << conv_reg_str(instr.rd) << ", " << instr.imm << "(sp)";
            break;
        case INSTRUCTION_TYPE_CL:
            os << instr_str << " " << conv_reg_str(instr.rd) << ", " << instr.imm << "(" << conv_reg_str(instr.rs1)
               << ")";
            break;
        case INSTRUCTION_TYPE_CS:
            os << instr_str << " " << conv_reg_str(instr.rs2) << ", " << instr.imm << "(" << conv_reg_str(instr.rs1)
               << ")";
            break;
        case INSTRUCTION_TYPE_CB:
            os << instr_str << " " << conv_reg_str(instr.rs1) << ", " << instr.imm;
            break;
        case INSTRUCTION_TYPE_CJ:
            os << instr_str << " " << instr.imm;
            break;
        default:
            throw IllegalInstruction("Unknown instruction type");
    }
    return os;
}
