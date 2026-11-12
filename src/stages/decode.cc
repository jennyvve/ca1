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
    REG_RD_EX = 0,
    REG_RD_MEM = 1,
} reg_rd_prev_e;

void InstructionDecodeStage::propagate() {
    if_id_int = if_id;
    instr = decoder.set_instr_reg(if_id.ir);

    regfile.setRS1(instr.rs1);
    regfile.setRS2(instr.rs2);
    imm = instr.imm;

    if (pipelining && instr.rs1 && instr.rs1 == reg_rd_prev[REG_RD_MEM]) {
        df_a = DATAFORWARD_MEM;
    }
    if (pipelining && instr.rs2 && instr.rs2 == reg_rd_prev[REG_RD_MEM]) {
        df_b = DATAFORWARD_MEM;
    }
    if (pipelining && instr.rs1 && instr.rs1 == reg_rd_prev[REG_RD_EX]) {
        df_a = DATAFORWARD_EX;
    }
    if (pipelining && instr.rs2 && instr.rs2 == reg_rd_prev[REG_RD_EX]) {
        df_b = DATAFORWARD_EX;
    }

    if (debug_mode && (!pipelining || (pipelining && if_id_int.npc != 0x0))) {
        auto storeFlags(std::cerr.flags());
        std::cerr << std::hex << std::showbase << if_id_int.npc << "\t";
        std::cerr.setf(storeFlags);
        std::cerr << decoder << std::endl;
    }
}

void InstructionDecodeStage::clock_pulse() {
    if (mux_flush.get_output() == true) {
        reg_rd_prev[REG_RD_EX] = reg_rd_prev[REG_RD_MEM] = 0;
        id_ex.flush();
    } else {
        id_ex.npc = if_id_int.npc;
        id_ex.ir = instr;
        id_ex.a = regfile.getReadData1();
        id_ex.b = regfile.getReadData2();
        id_ex.imm = imm;
        id_ex.df_a = df_a;
        id_ex.df_b = df_b;
        reg_rd_prev[REG_RD_MEM] = reg_rd_prev[REG_RD_EX];
        reg_rd_prev[REG_RD_EX] = instr.rd;
        if (!pipelining || (pipelining && if_id_int.npc != 0x0) || if_id.ir != TestEndMarker) {
            ++n_instr_issued;
        }
    }

    instr = {};
    imm = 0;
    df_a = df_b = DATAFORWARD_NONE;
    if_id_int = {};
}
