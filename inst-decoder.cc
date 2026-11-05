/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    inst-decoder.cc - RISC-V instruction decoder.
 *
 * Copyright (C) 2016,2019  Leiden University, The Netherlands.
 *
 */

#include "inst-decoder.h"

#include <iostream>
#include <map>

/*
 * Class InstructionDecoder -- helper class for getting specific
 * information from the decoded instruction.
 */

std::map<instruction_opcode_e, instruction_type_e> instruction_type_map_32 = {
    {INSTRUCTION_OPCODE_OP_IMM, INSTRUCTION_TYPE_I}, {INSTRUCTION_OPCODE_OP_IMM_32, INSTRUCTION_TYPE_I},
    {INSTRUCTION_OPCODE_LOAD, INSTRUCTION_TYPE_I},   {INSTRUCTION_OPCODE_MISC_MEM, INSTRUCTION_TYPE_I},
    {INSTRUCTION_OPCODE_AUIPC, INSTRUCTION_TYPE_U},  {INSTRUCTION_OPCODE_STORE, INSTRUCTION_TYPE_S},
    {INSTRUCTION_OPCODE_OP, INSTRUCTION_TYPE_R},     {INSTRUCTION_OPCODE_OP_32, INSTRUCTION_TYPE_R},
    {INSTRUCTION_OPCODE_LUI, INSTRUCTION_TYPE_U},    {INSTRUCTION_OPCODE_BRANCH, INSTRUCTION_TYPE_B},
    {INSTRUCTION_OPCODE_JALR, INSTRUCTION_TYPE_I},   {INSTRUCTION_OPCODE_JAL, INSTRUCTION_TYPE_J},
};

std::map<std::pair<instruction_opcode_e, instruction_funct3_e>, instruction_type_e> instruction_type_map_16 = {
    {{INSTRUCTION_OPCODE_C0, INSTRUCTION_FUNCT3_C0}, INSTRUCTION_TYPE_CIW},
    {{INSTRUCTION_OPCODE_C0, INSTRUCTION_FUNCT3_C2}, INSTRUCTION_TYPE_CL},
    {{INSTRUCTION_OPCODE_C0, INSTRUCTION_FUNCT3_C6}, INSTRUCTION_TYPE_CS},
    {{INSTRUCTION_OPCODE_C1, INSTRUCTION_FUNCT3_C0}, INSTRUCTION_TYPE_CI},
    {{INSTRUCTION_OPCODE_C1, INSTRUCTION_FUNCT3_C1}, INSTRUCTION_TYPE_CI},
    {{INSTRUCTION_OPCODE_C1, INSTRUCTION_FUNCT3_C2}, INSTRUCTION_TYPE_CI},
    {{INSTRUCTION_OPCODE_C1, INSTRUCTION_FUNCT3_C3}, INSTRUCTION_TYPE_CI},
    {{INSTRUCTION_OPCODE_C1, INSTRUCTION_FUNCT3_C5}, INSTRUCTION_TYPE_CJ},
    {{INSTRUCTION_OPCODE_C1, INSTRUCTION_FUNCT3_C6}, INSTRUCTION_TYPE_CB},
    {{INSTRUCTION_OPCODE_C1, INSTRUCTION_FUNCT3_C7}, INSTRUCTION_TYPE_CB},
    {{INSTRUCTION_OPCODE_C2, INSTRUCTION_FUNCT3_C0}, INSTRUCTION_TYPE_CI},
    {{INSTRUCTION_OPCODE_C2, INSTRUCTION_FUNCT3_C2}, INSTRUCTION_TYPE_CI},
    {{INSTRUCTION_OPCODE_C2, INSTRUCTION_FUNCT3_C4}, INSTRUCTION_TYPE_CR},
    {{INSTRUCTION_OPCODE_C2, INSTRUCTION_FUNCT3_C6}, INSTRUCTION_TYPE_CSS},
};

int32_t sign_extend(uint32_t value, unsigned width) {
    uint32_t sign_bit = 1u << (width - 1);
    return static_cast<int32_t>((value ^ sign_bit) - sign_bit);
}

void InstructionDecoder::dec_instr() {
    instruction_raw_32_u raw = {.word = instr_reg};

    this->instr_struct.type = instruction_type_map_32[(instruction_opcode_e)raw.type_r.opcode];
    this->instr_struct.opcode = (instruction_opcode_e)raw.type_r.opcode;
    this->instr_struct.rd = raw.type_r.rd;
    this->instr_struct.rs1 = raw.type_r.rs1;
    this->instr_struct.rs2 = raw.type_r.rs2;

    switch (this->instr_struct.type) {
        case INSTRUCTION_TYPE_I:
            this->instr_struct.imm = sign_extend(raw.type_i.imm, 12);
            break;
        case INSTRUCTION_TYPE_S: {
            const uint32_t imm = (raw.type_s.imm_11_5 << 5) | raw.type_s.im_4_0;
            this->instr_struct.imm = sign_extend(imm, 12);
            break;
        }
        case INSTRUCTION_TYPE_B: {
            const uint32_t imm = ((raw.type_b.imm_12_10_5 >> 6) << 12) | ((raw.type_b.im_4_1_11 & 0x1u) << 11) |
                                 ((raw.type_b.imm_12_10_5 & 0x3Fu) << 5) | (((raw.type_b.im_4_1_11 >> 1) & 0xFu) << 1);
            this->instr_struct.imm = sign_extend(imm, 13);
            break;
        }
        case INSTRUCTION_TYPE_U:
            this->instr_struct.imm = static_cast<int32_t>(raw.type_u.imm << 12);
            break;
        case INSTRUCTION_TYPE_J: {
            const uint32_t imm = (((raw.type_j.imm >> 19) & 0x1u) << 20) | ((raw.type_j.imm & 0xFFu) << 12) |
                                 (((raw.type_j.imm >> 8) & 0x1u) << 11) | (((raw.type_j.imm >> 9) & 0x3FFu) << 1);
            this->instr_struct.imm = sign_extend(imm, 21);
            break;
        }
        default:
            this->instr_struct.imm = 0;
            break;
    }

    switch (this->instr_struct.type) {
        case INSTRUCTION_TYPE_R:
            this->instr_struct.funct3 = (instruction_funct3_e)raw.type_r.funct3;
            this->instr_struct.funct7 = (instruction_funct7_e)raw.type_r.funct7;
            break;
        case INSTRUCTION_TYPE_I:
        case INSTRUCTION_TYPE_S:
        case INSTRUCTION_TYPE_B:
            this->instr_struct.funct3 = (instruction_funct3_e)raw.type_r.funct3;
            this->instr_struct.funct7 = INSTRUCTION_FUNCT7_NONE;
            break;
        default:
            this->instr_struct.funct3 = INSTRUCTION_FUNCT3_NONE;
            this->instr_struct.funct7 = INSTRUCTION_FUNCT7_NONE;
            break;
    }
}

void InstructionDecoder::dec_instr_compressed() {
    instruction_raw_16_u raw = {.word = instr_reg};
    this->instr_struct.type =
        instruction_type_map_16[{(instruction_opcode_e)raw.type_i.opcode, (instruction_funct3_e)raw.type_i.funct3}];
    this->instr_struct.opcode = (instruction_opcode_e)raw.type_i.opcode;
    this->instr_struct.funct3 = (instruction_funct3_e)raw.type_i.funct3;
    this->instr_struct.imm = 0;
    this->instr_struct.rd = 0;
    this->instr_struct.rs1 = 0;
    this->instr_struct.rs2 = 0;
    this->instr_struct.offset = 0;
    this->instr_struct.target = 0;

    switch (this->instr_struct.type) {
        case INSTRUCTION_TYPE_CR:
            this->instr_struct.rd = raw.type_r.rd_rs1;
            this->instr_struct.rs1 = raw.type_r.rd_rs1;
            this->instr_struct.rs2 = raw.type_r.rs2;
            break;

        case INSTRUCTION_TYPE_CI:
            this->instr_struct.rd = raw.type_i.rd_rs1;
            this->instr_struct.rs1 = raw.type_i.rd_rs1;
            this->instr_struct.imm = (raw.type_i.imm_1 << 5) | raw.type_i.imm_0;
            break;

        case INSTRUCTION_TYPE_CSS:
            this->instr_struct.rs2 = raw.type_ss.rs2;
            this->instr_struct.imm = raw.type_ss.imm;
            break;

        case INSTRUCTION_TYPE_CIW:
            this->instr_struct.rd = raw.type_iw.rd;
            this->instr_struct.imm = raw.type_iw.imm;
            break;

        case INSTRUCTION_TYPE_CL:
            this->instr_struct.rd = raw.type_l.rd;
            this->instr_struct.rs1 = raw.type_l.rs1;
            this->instr_struct.imm = (raw.type_l.imm_1 << 2) | raw.type_l.imm_0;
            break;

        case INSTRUCTION_TYPE_CS:
            this->instr_struct.rs1 = raw.type_s.rs1;
            this->instr_struct.rs2 = raw.type_s.rs2;
            this->instr_struct.imm = (raw.type_s.imm_1 << 2) | raw.type_s.imm_0;
            break;

        case INSTRUCTION_TYPE_CB:
            this->instr_struct.rs1 = raw.type_b.rs1;
            this->instr_struct.offset = (raw.type_b.off_1 << 2) | raw.type_b.off_0;
            break;

        case INSTRUCTION_TYPE_CJ:
            this->instr_struct.target = raw.type_j.target;
            break;
        default:
            break;
    }
}

instruction_t InstructionDecoder::set_instr_reg(const uint32_t instr_reg) {
    this->instr_reg = instr_reg;

    if ((instr_reg & 0x3u) != 0x3u) {
        this->dec_instr_compressed();
    } else {
        this->dec_instr();
    }

    return this->instr_struct;
}

uint32_t InstructionDecoder::get_instr_reg() const { return instr_reg; }

instruction_t InstructionDecoder::get_instr() const { return this->instr_struct; }
