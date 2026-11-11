/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    inst-decoder.h - RISC-V instruction decoder.
 *
 * Copyright (C) 2016,2019  Leiden University, The Netherlands.
 */

#ifndef __INST_DECODER_H__
#define __INST_DECODER_H__

#include <cstdint>
#include <stdexcept>

#include "reg-file.h"

/* TODO: add enums and constants necessary for your instruction
 * decoder. */

/* Exception that should be thrown when an illegal instruction
 * is encountered.
 */
class IllegalInstruction : public std::runtime_error {
   public:
    explicit IllegalInstruction(const std::string &what) : std::runtime_error(what) {}

    explicit IllegalInstruction(const char *what) : std::runtime_error(what) {}
};

typedef union {
    struct {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t funct7 : 7;
    } type_r;
    struct {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t imm : 12;
    } type_i;
    struct {
        uint32_t opcode : 7;
        uint32_t im_4_0 : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t imm_11_5 : 7;
    } type_s;
    struct {
        uint32_t opcode : 7;
        uint32_t im_4_1_11 : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t imm_12_10_5 : 7;
    } type_b;
    struct {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t imm : 20;
    } type_u;
    struct {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t imm : 20;
    } type_j;
    uint32_t word;
} instruction_raw_32_u;

typedef union {
    struct {
        uint32_t opcode : 2;
        uint32_t rs2 : 5;
        uint32_t rd_rs1 : 5;
        uint32_t funct4 : 4;
    } type_r;
    struct {
        uint32_t opcode : 2;
        uint32_t imm_0 : 5;
        uint32_t rd_rs1 : 5;
        uint32_t imm_1 : 1;
        uint32_t funct3 : 3;
    } type_i;
    struct {
        uint32_t opcode : 2;
        uint32_t rs2 : 5;
        uint32_t imm : 7;
        uint32_t funct3 : 3;
    } type_ss;
    struct {
        uint32_t opcode : 2;
        uint32_t rd : 3;
        uint32_t imm : 8;
        uint32_t funct3 : 3;
    } type_iw;
    struct {
        uint32_t opcode : 2;
        uint32_t rd : 3;
        uint32_t imm_0 : 2;
        uint32_t rs1 : 3;
        uint32_t imm_1 : 3;
        uint32_t funct3 : 3;
    } type_l;
    struct {
        uint32_t opcode : 2;
        uint32_t rs2 : 3;
        uint32_t imm_0 : 2;
        uint32_t rs1 : 3;
        uint32_t imm_1 : 3;
        uint32_t funct3 : 3;
    } type_s;
    struct {
        uint32_t opcode : 2;
        uint32_t off_0 : 2;
        uint32_t rs1 : 3;
        uint32_t off_1 : 3;
        uint32_t funct3 : 3;
    } type_b;
    struct {
        uint32_t opcode : 2;
        uint32_t target : 11;
        uint32_t funct3 : 3;
    } type_j;
    uint32_t word;
} instruction_raw_16_u;

typedef enum {
    INSTRUCTION_TYPE_R,
    INSTRUCTION_TYPE_B,
    INSTRUCTION_TYPE_I,
    INSTRUCTION_TYPE_S,
    INSTRUCTION_TYPE_U,
    INSTRUCTION_TYPE_J,

} instruction_type_e;

typedef enum {
    INSTRUCTION_OPCODE_OP_IMM = 0x13,
    INSTRUCTION_OPCODE_OP_IMM_32 = 0x1B,
    INSTRUCTION_OPCODE_LOAD = 0x03,
    INSTRUCTION_OPCODE_MISC_MEM = 0x0F,
    INSTRUCTION_OPCODE_AUIPC = 0x17,
    INSTRUCTION_OPCODE_STORE = 0x23,
    INSTRUCTION_OPCODE_OP = 0x33,
    INSTRUCTION_OPCODE_OP_32 = 0x3B,
    INSTRUCTION_OPCODE_LUI = 0x37,
    INSTRUCTION_OPCODE_BRANCH = 0x63,
    INSTRUCTION_OPCODE_JALR = 0x67,
    INSTRUCTION_OPCODE_JAL = 0x6F,
    INSTRUCTION_OPCODE_SYSTEM = 0x73,

} instruction_opcode_e;

typedef enum {
    INSTRUCTION_FUNCT3_NONE = 0X00,
    INSTRUCTION_FUNCT3_LOAD_LB = 0X00,
    INSTRUCTION_FUNCT3_LOAD_LH = 0X01,
    INSTRUCTION_FUNCT3_LOAD_LW = 0X02,
    INSTRUCTION_FUNCT3_LOAD_LD = 0X03,
    INSTRUCTION_FUNCT3_LOAD_LBU = 0X04,
    INSTRUCTION_FUNCT3_LOAD_LHU = 0X05,
    INSTRUCTION_FUNCT3_STORE_SB = 0X00,
    INSTRUCTION_FUNCT3_STORE_SH = 0X01,
    INSTRUCTION_FUNCT3_STORE_SW = 0X02,
    INSTRUCTION_FUNCT3_STORE_SD = 0X03,
    INSTRUCTION_FUNCT3_OP_IM_ADDI = 0X00,
    INSTRUCTION_FUNCT3_OP_IM_SLTI = 0X02,
    INSTRUCTION_FUNCT3_OP_IM_SLTIU = 0X03,
    INSTRUCTION_FUNCT3_OP_IM_XORI = 0X04,
    INSTRUCTION_FUNCT3_OP_IM_ORI = 0X06,
    INSTRUCTION_FUNCT3_OP_IM_ANDI = 0X07,
    INSTRUCTION_FUNCT3_OP_IM_SLLI = 0X01,
    INSTRUCTION_FUNCT3_OP_IM_SRLI_SRAI = 0X05,
    INSTRUCTION_FUNCT3_OP_32_ADDW_SUBW = 0X00,
    INSTRUCTION_FUNCT3_OP_32_SLLW = 0X01,
    INSTRUCTION_FUNCT3_OP_32_SRLW_SRAW = 0X05,
    INSTRUCTION_FUNCT3_OP_IM_32_ADDIW = 0X00,
    INSTRUCTION_FUNCT3_OP_IM_32_SLLIW = 0X01,
    INSTRUCTION_FUNCT3_OP_IM_32_SRLIW_SRAIW = 0X05,
    INSTRUCTION_FUNCT3_OP_ADD_SUB = 0X00,
    INSTRUCTION_FUNCT3_OP_SLL = 0X01,
    INSTRUCTION_FUNCT3_OP_SLT = 0X02,
    INSTRUCTION_FUNCT3_OP_SLTU = 0X03,
    INSTRUCTION_FUNCT3_OP_XOR = 0X04,
    INSTRUCTION_FUNCT3_OP_SRL_SRA = 0X05,
    INSTRUCTION_FUNCT3_OP_OR = 0X06,
    INSTRUCTION_FUNCT3_OP_AND = 0X07,
    INSTRUCTION_FUNCT3_BRANCH_BEQ = 0X00,
    INSTRUCTION_FUNCT3_BRANCH_BNE = 0X01,
    INSTRUCTION_FUNCT3_BRANCH_BLT = 0X04,
    INSTRUCTION_FUNCT3_BRANCH_BGE = 0X05,
    INSTRUCTION_FUNCT3_BRANCH_BLTU = 0X06,
    INSTRUCTION_FUNCT3_BRANCH_BGEU = 0X07,
    INSTRUCTION_FUNCT3_JALR = 0X00,
    INSTRUCTION_FUNCT3_C0 = 0X00,
    INSTRUCTION_FUNCT3_C1 = 0X01,
    INSTRUCTION_FUNCT3_C2 = 0X02,
    INSTRUCTION_FUNCT3_C3 = 0X03,
    INSTRUCTION_FUNCT3_C4 = 0X04,
    INSTRUCTION_FUNCT3_C5 = 0X05,
    INSTRUCTION_FUNCT3_C6 = 0X06,
    INSTRUCTION_FUNCT3_C7 = 0X07,
} instruction_funct3_e;

typedef enum {
    INSTRUCTION_FUNCT7_NONE = 0X00,
    INSTRUCTION_FUNCT7_OP_ADD = 0X00,
    INSTRUCTION_FUNCT7_OP_SLL = 0X00,
    INSTRUCTION_FUNCT7_OP_SLT = 0X00,
    INSTRUCTION_FUNCT7_OP_SLTU = 0X00,
    INSTRUCTION_FUNCT7_OP_XOR = 0X00,
    INSTRUCTION_FUNCT7_OP_SRL = 0X00,
    INSTRUCTION_FUNCT7_OP_OR = 0X00,
    INSTRUCTION_FUNCT7_OP_AND = 0X00,
    INSTRUCTION_FUNCT7_OP_SUB = 0X20,
    INSTRUCTION_FUNCT7_OP_SRA = 0X20,
    INSTRUCTION_FUNCT7_OP_IMM_32_SLLIW = 0X00,
    INSTRUCTION_FUNCT7_OP_IMM_32_SRLIW = 0X00,
    INSTRUCTION_FUNCT7_OP_IMM_32_SRAIW = 0X20,
    INSTRUCTION_FUNCT7_OP_32_ADDW = 0X00,
    INSTRUCTION_FUNCT7_OP_32_SUBW = 0X20,
    INSTRUCTION_FUNCT7_OP_32_SLLW = 0X00,
    INSTRUCTION_FUNCT7_OP_32_SRLW = 0X00,
    INSTRUCTION_FUNCT7_OP_32_SRAW = 0X20,
} instruction_funct7_e;

typedef struct {
    instruction_type_e type;
    instruction_opcode_e opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    instruction_funct3_e funct3;
    instruction_funct7_e funct7;
    int32_t imm;
    uint32_t offset;
    uint32_t target;
} instruction_t;

const uint32_t instr_nop_raw = 0x13;

/* InstructionDecoder component to be used by class Processor */
class InstructionDecoder {
   public:
    instruction_t set_instr_reg(const uint32_t instr_reg);

    uint32_t get_instr_reg() const;
    instruction_t get_instr() const;

   private:
    uint32_t instr_reg;
    instruction_t instr_struct;
    void dec_instr();
    void dec_instr_compressed();
};

std::ostream &operator<<(std::ostream &os, const InstructionDecoder &decoder);

#endif /* __INST_DECODER_H__ */
