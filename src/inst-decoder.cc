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
            if (instr_struct.imm > (524287 << 12)) {
                throw IllegalInstruction("U type instruction has a value exceeding 524287.");
            }
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
        case INSTRUCTION_TYPE_I:  // todo?
            this->instr_struct.funct3 = (instruction_funct3_e)raw.type_r.funct3;
            this->instr_struct.funct7 = (instruction_funct7_e)raw.type_r.funct7;
            break;
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

instruction_t InstructionDecoder::set_instr_reg(const uint32_t instr_reg) {
    if (instr_reg == TestEndMarker) {
        this->instr_reg = instr_nop_raw;
    } else {
        this->instr_reg = instr_reg;
    }
    this->dec_instr();
    return this->instr_struct;
}

uint32_t InstructionDecoder::get_instr_reg() const { return instr_reg; }

instruction_t InstructionDecoder::get_instr() const { return this->instr_struct; }
