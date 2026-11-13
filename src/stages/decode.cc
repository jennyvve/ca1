/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    decoder.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include <iostream>

#include "../stages.h"

void dump_instruction(std::ostream &os, const uint32_t instr_reg, const InstructionDecoder &decoder);

typedef enum {
    REG_RD_PREV_1 = 0,
    REG_RD_PREV_2 = 1,
} reg_rd_prev_e;

std::map<instruction_opcode_e, dataforward_type_e> opcode_to_type_map = {
    {INSTRUCTION_OPCODE_JALR, DATAFORWARD_TYPE_EX},   {INSTRUCTION_OPCODE_JAL, DATAFORWARD_TYPE_EX},
    {INSTRUCTION_OPCODE_AUIPC, DATAFORWARD_TYPE_EX},  {INSTRUCTION_OPCODE_LUI, DATAFORWARD_TYPE_EX},
    {INSTRUCTION_OPCODE_OP, DATAFORWARD_TYPE_EX},     {INSTRUCTION_OPCODE_OP_IMM, DATAFORWARD_TYPE_EX},
    {INSTRUCTION_OPCODE_OP_32, DATAFORWARD_TYPE_EX},  {INSTRUCTION_OPCODE_OP_IMM_32, DATAFORWARD_TYPE_EX},
    {INSTRUCTION_OPCODE_LOAD, DATAFORWARD_TYPE_LOAD}, {INSTRUCTION_OPCODE_STORE, DATAFORWARD_TYPE_EX},
    {INSTRUCTION_OPCODE_BRANCH, DATAFORWARD_TYPE_EX},
};

void InstructionDecodeStage::process_instr(uint32_t ir) {
    instr = decoder.set_instr_reg(ir);
    regfile.setRS1(instr.rs1);
    regfile.setRS2(instr.rs2);
    imm = instr.imm;
}

void InstructionDecodeStage::insert_nop() {
    nop_inserted = true;
    if_id_int_prev = if_id_int;
    if_id_int.npc = 0;
    if_id_int.ir = instr_nop_raw;

    process_instr(instr_nop_raw);

    mux_pc.set_selector(InputSelectorPC::STAY);
}

void InstructionDecodeStage::dataforward_detect() {
    if (!pipelining) {
        return;
    }

    bool df_a_prev_1 = (instr.rs1 && instr.rs1 == prev_instr[REG_RD_PREV_1].rd);
    bool df_b_prev_1 = (instr.rs2 && instr.rs2 == prev_instr[REG_RD_PREV_1].rd);
    bool df_a_prev_2 = (instr.rs1 && instr.rs1 == prev_instr[REG_RD_PREV_2].rd);
    bool df_b_prev_2 = (instr.rs2 && instr.rs2 == prev_instr[REG_RD_PREV_2].rd);

    if (prev_instr[REG_RD_PREV_1].type == DATAFORWARD_TYPE_LOAD && (df_a_prev_1 || df_b_prev_1)) {
        insert_nop();
    }

    if (df_a_prev_2) {
        df_a = {.type = prev_instr[REG_RD_PREV_2].type, .pipe = DATAFORWARD_PIPE_MEM};
    }
    if (df_b_prev_2) {
        df_b = {.type = prev_instr[REG_RD_PREV_2].type, .pipe = DATAFORWARD_PIPE_MEM};
    }
    if (df_a_prev_1) {
        df_a = {.type = prev_instr[REG_RD_PREV_1].type, .pipe = DATAFORWARD_PIPE_EX};
    }
    if (df_b_prev_1) {
        df_b = {.type = prev_instr[REG_RD_PREV_1].type, .pipe = DATAFORWARD_PIPE_EX};
    }
}

void InstructionDecodeStage::propagate() {
    if (nop_inserted) {
        if_id_int = if_id_int_prev;
        nop_inserted = false;
    } else {
        if_id_int = if_id;
    }

    process_instr(if_id_int.ir);
    dataforward_detect();

    if (debug_mode && (!pipelining || (pipelining && if_id_int.npc != 0x0))) {
        auto storeFlags(std::cerr.flags());
        std::cerr << std::hex << std::showbase << if_id_int.npc << "\t";
        std::cerr.setf(storeFlags);
        std::cerr << decoder << std::endl;
    }
}

void InstructionDecodeStage::clock_pulse() {
    if (mux_flush.get_output() == true) {
        prev_instr[REG_RD_PREV_1] = prev_instr[REG_RD_PREV_2] = {};
        id_ex.flush();
    } else {
        id_ex.npc = if_id_int.npc;
        id_ex.ir = instr;
        id_ex.a = regfile.getReadData1();
        id_ex.b = regfile.getReadData2();
        id_ex.imm = imm;
        id_ex.df_a = df_a;
        id_ex.df_b = df_b;
        prev_instr[REG_RD_PREV_2] = prev_instr[REG_RD_PREV_1];
        prev_instr[REG_RD_PREV_1] = {.rd = instr.rd, .type = opcode_to_type_map[instr.opcode]};
        if (!pipelining || (pipelining && if_id_int.npc != 0x0) || if_id.ir != TestEndMarker) {
            ++n_instr_issued;
        }
    }

    instr = {};
    imm = 0;
    df_a = df_b = {};
    if_id_int = {};
}
