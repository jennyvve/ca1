/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    exec.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include <iostream>

#include "../stages.h"

static std::map<std::pair<instruction_funct3_e, instruction_funct7_e>, ALUOP> map_instr_aluop = {
    {{INSTRUCTION_FUNCT3_OP_ADD_SUB, INSTRUCTION_FUNCT7_OP_ADD}, ALUOP::ADD},
    {{INSTRUCTION_FUNCT3_OP_ADD_SUB, INSTRUCTION_FUNCT7_OP_SUB}, ALUOP::SUB},
    {{INSTRUCTION_FUNCT3_OP_SLL, INSTRUCTION_FUNCT7_OP_SLL}, ALUOP::SLL},
    {{INSTRUCTION_FUNCT3_OP_SLT, INSTRUCTION_FUNCT7_OP_SLT}, ALUOP::SLT},
    {{INSTRUCTION_FUNCT3_OP_SLTU, INSTRUCTION_FUNCT7_OP_SLTU}, ALUOP::SLTU},
    {{INSTRUCTION_FUNCT3_OP_XOR, INSTRUCTION_FUNCT7_OP_XOR}, ALUOP::XOR},
    {{INSTRUCTION_FUNCT3_OP_SRL_SRA, INSTRUCTION_FUNCT7_OP_SRL}, ALUOP::SRL},
    {{INSTRUCTION_FUNCT3_OP_SRL_SRA, INSTRUCTION_FUNCT7_OP_SRA}, ALUOP::SRA},
    {{INSTRUCTION_FUNCT3_OP_OR, INSTRUCTION_FUNCT7_OP_OR}, ALUOP::OR},
    {{INSTRUCTION_FUNCT3_OP_AND, INSTRUCTION_FUNCT7_OP_AND}, ALUOP::AND},
};

static std::map<instruction_funct3_e, ALUOP> map_instr_aluop_imm = {
    {INSTRUCTION_FUNCT3_OP_IM_ADDI, ALUOP::ADD}, {INSTRUCTION_FUNCT3_OP_IM_SLLI, ALUOP::SLL},
    {INSTRUCTION_FUNCT3_OP_IM_SLTI, ALUOP::SLT}, {INSTRUCTION_FUNCT3_OP_IM_SLTIU, ALUOP::SLTU},
    {INSTRUCTION_FUNCT3_OP_IM_XORI, ALUOP::XOR}, {INSTRUCTION_FUNCT3_OP_IM_SRLI_SRAI, ALUOP::SRA},
    {INSTRUCTION_FUNCT3_OP_OR, ALUOP::OR},       {INSTRUCTION_FUNCT3_OP_IM_ANDI, ALUOP::AND},
};

void ExecuteStage::exec_handler_op_imm(void) {
    ALUOP op = map_instr_aluop_imm[id_ex_int.ir.funct3];
    alu.set_a(id_ex_int.a);
    alu.set_b(id_ex_int.imm);
    alu.set_op(op);
    alu_output = alu.getResult();
}

void ExecuteStage::exec_handler_op(void) {
    ALUOP op = map_instr_aluop[{id_ex_int.ir.funct3, id_ex_int.ir.funct7}];
    alu.set_a(id_ex_int.a);
    alu.set_b(id_ex_int.b);
    alu.set_op(op);
    alu_output = alu.getResult();
}

static std::map<std::pair<instruction_funct3_e, instruction_funct7_e>, ALUOP> map_instr_aluop_32 = {
    {{INSTRUCTION_FUNCT3_OP_32_ADDW_SUBW, INSTRUCTION_FUNCT7_OP_32_ADDW}, ALUOP::ADD_32},
    {{INSTRUCTION_FUNCT3_OP_32_ADDW_SUBW, INSTRUCTION_FUNCT7_OP_32_SUBW}, ALUOP::SUB_32},
    {{INSTRUCTION_FUNCT3_OP_32_SLLW, INSTRUCTION_FUNCT7_OP_32_SLLW}, ALUOP::SLL_32},
    {{INSTRUCTION_FUNCT3_OP_32_SRLW_SRAW, INSTRUCTION_FUNCT7_OP_32_SRLW}, ALUOP::SRL_32},
    {{INSTRUCTION_FUNCT3_OP_32_SRLW_SRAW, INSTRUCTION_FUNCT7_OP_32_SRAW}, ALUOP::SRA_32},
};

static std::map<instruction_funct3_e, ALUOP> map_instr_aluop_imm_32 = {
    {INSTRUCTION_FUNCT3_OP_IM_32_ADDIW, ALUOP::ADD_32},
};
void ExecuteStage::exec_handler_op_imm_32(void) {
    ALUOP op;
    if (map_instr_aluop_imm_32.find(id_ex_int.ir.funct3) != map_instr_aluop_imm_32.end()) {
        op = map_instr_aluop_imm_32[id_ex_int.ir.funct3];
    } else {
        op = map_instr_aluop_32[{id_ex_int.ir.funct3, id_ex_int.ir.funct7}];
    }
    alu.set_a(id_ex_int.a);
    alu.set_b(id_ex_int.imm);
    alu.set_op(op);
    alu_output = alu.getResult();
}

void ExecuteStage::exec_handler_op_32(void) {
    ALUOP op = map_instr_aluop_32[{id_ex_int.ir.funct3, id_ex_int.ir.funct7}];
    alu.set_a(id_ex_int.a);
    alu.set_b(id_ex_int.b);
    alu.set_op(op);
    alu_output = alu.getResult();
}

std::map<instruction_funct3_e, ALUOP> map_branch_aluop = {
    {INSTRUCTION_FUNCT3_BRANCH_BEQ, ALUOP::BEQ},   {INSTRUCTION_FUNCT3_BRANCH_BNE, ALUOP::BNE},
    {INSTRUCTION_FUNCT3_BRANCH_BLT, ALUOP::BLT},   {INSTRUCTION_FUNCT3_BRANCH_BGE, ALUOP::BGE},
    {INSTRUCTION_FUNCT3_BRANCH_BLTU, ALUOP::BLTU}, {INSTRUCTION_FUNCT3_BRANCH_BGEU, ALUOP::BGEU},
};

void ExecuteStage::exec_handler_branch(void) {
    ALUOP op = map_branch_aluop[id_ex_int.ir.funct3];
    alu.set_a(id_ex_int.a);
    alu.set_b(id_ex_int.b);
    alu.set_op(op);
    cond = alu.getResult();
    alu_output = id_ex_int.npc + id_ex_int.imm;
}

void ExecuteStage::exec_handler_lui(void) { alu_output = id_ex_int.imm; }
void ExecuteStage::exec_handler_auipc(void) { alu_output = id_ex_int.npc + static_cast<int64_t>(id_ex_int.imm); }
void ExecuteStage::exec_handler_jal(void) { alu_output = id_ex_int.npc + static_cast<int64_t>(id_ex_int.imm); }
void ExecuteStage::exec_handler_jalr(void) { alu_output = ((id_ex_int.a + static_cast<int64_t>(id_ex_int.imm)) & ~1); }

void ExecuteStage::exec_handler_mem(void) { alu_output = (RegValue)((int64_t)id_ex_int.a + (int64_t)id_ex_int.imm); }

void ExecuteStage::propagate() {
    id_ex_int = id_ex;

    if (id_ex_int.df_a != DATAFORWARD_NONE) {
        id_ex_int.a = dataforward_pipeline[id_ex_int.df_a];
    }
    if (id_ex_int.df_b != DATAFORWARD_NONE) {
        id_ex_int.b = dataforward_pipeline[id_ex_int.df_b];
    }

    if (exec_map.find(id_ex_int.ir.opcode) != exec_map.end()) {
        (this->*exec_map[id_ex_int.ir.opcode])();
    }
}

void ExecuteStage::clock_pulse() {
    if (mux_flush.get_output() == true) {
        dataforward_pipeline[DATAFORWARD_MEM] = dataforward_pipeline[DATAFORWARD_EX] = 0;
        ex_m.flush();
    } else {
        ex_m.npc = id_ex_int.npc;
        ex_m.ir = id_ex_int.ir;
        ex_m.a = id_ex_int.a;
        ex_m.b = id_ex_int.b;
        ex_m.imm = id_ex_int.imm;
        ex_m.cond = cond;
        ex_m.alu_output = alu_output;
        dataforward_pipeline[DATAFORWARD_MEM] = dataforward_pipeline[DATAFORWARD_EX];
        dataforward_pipeline[DATAFORWARD_EX] = alu_output;
    }
    cond = alu_output = 0;
    id_ex_int = {};
}
