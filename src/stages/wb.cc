/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    wb.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include <iostream>

#include "../stages.h"

void WriteBackStage::wb_handler_op(void) {
    regfile.setRD(m_wb.ir.rd);
    regfile.setWriteData(m_wb.alu_output);
    regfile.setWriteEnable(true);
    regfile.clock_pulse();
    regfile.setWriteEnable(false);
}

void WriteBackStage::wb_handler_load(void) {
    regfile.setRD(m_wb.ir.rd);
    regfile.setWriteData(m_wb.lmd);
    regfile.setWriteEnable(true);
    regfile.clock_pulse();
    regfile.setWriteEnable(false);
}

void WriteBackStage::propagate() {
    if (!pipelining || (pipelining && m_wb.npc)) {
        ++n_instr_completed;
    }

    if (wb_map.find(m_wb.ir.opcode) != wb_map.end()) {
        (this->*wb_map[m_wb.ir.opcode])();
    }
}

void WriteBackStage::clock_pulse() {}
