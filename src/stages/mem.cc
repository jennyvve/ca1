/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    mem.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include <iostream>

#include "../stages.h"

void MemoryStage::mem_handler_branch(void) {
    if (ex_m_int.cond) {
        mux_pc.set_input(InputSelectorPC::BRANCH, ex_m_int.alu_output);
        mux_pc.set_selector(InputSelectorPC::BRANCH);
        mux_flush.set_selector(InputSelectorFlush::YES);
    }
}

void MemoryStage::mem_handler_jal(void) {
    mux_pc.set_input(InputSelectorPC::BRANCH, ex_m_int.alu_output);
    mux_pc.set_selector(InputSelectorPC::BRANCH);
    mux_flush.set_selector(InputSelectorFlush::YES);
    ex_m_int.alu_output = ex_m_int.npc + INSTR_SIZE;
}

std::map<instruction_funct3_e, bool> map_mem_sign_ext = {
    {INSTRUCTION_FUNCT3_LOAD_LB, true},  {INSTRUCTION_FUNCT3_LOAD_LH, true},   {INSTRUCTION_FUNCT3_LOAD_LW, true},
    {INSTRUCTION_FUNCT3_LOAD_LD, false}, {INSTRUCTION_FUNCT3_LOAD_LBU, false}, {INSTRUCTION_FUNCT3_LOAD_LHU, false},
};

std::map<instruction_funct3_e, uint8_t> map_mem_size = {
    {INSTRUCTION_FUNCT3_LOAD_LB, 1},  {INSTRUCTION_FUNCT3_LOAD_LH, 2},  {INSTRUCTION_FUNCT3_LOAD_LW, 4},
    {INSTRUCTION_FUNCT3_LOAD_LD, 8},  {INSTRUCTION_FUNCT3_LOAD_LBU, 1}, {INSTRUCTION_FUNCT3_LOAD_LHU, 2},
    {INSTRUCTION_FUNCT3_STORE_SB, 1}, {INSTRUCTION_FUNCT3_STORE_SH, 2}, {INSTRUCTION_FUNCT3_STORE_SW, 4},
    {INSTRUCTION_FUNCT3_STORE_SD, 8},
};

void MemoryStage::mem_handler_load(void) {
    data_memory.setSize(map_mem_size[ex_m_int.ir.funct3]);
    data_memory.setAddress(ex_m_int.alu_output);
    data_memory.setReadEnable(true);
    lmd = data_memory.getDataOut(map_mem_sign_ext[ex_m_int.ir.funct3]);
}

void MemoryStage::mem_handler_store(void) {
    data_memory.setSize(map_mem_size[ex_m_int.ir.funct3]);
    data_memory.setAddress(ex_m_int.alu_output);
    data_memory.setDataIn(ex_m_int.b);
    data_memory.setWriteEnable(true);
}

void MemoryStage::propagate() {
    ex_m_int = ex_m;
    if (mem_map.find(ex_m_int.ir.opcode) != mem_map.end()) {
        (this->*mem_map[ex_m_int.ir.opcode])();
    }
}

void MemoryStage::clock_pulse() {
    data_memory.clock_pulse();
    data_memory.setReadEnable(false);
    data_memory.setWriteEnable(false);

    m_wb.npc = ex_m_int.npc;
    m_wb.ir = ex_m_int.ir;
    m_wb.a = ex_m_int.a;
    m_wb.b = ex_m_int.b;
    m_wb.imm = ex_m_int.imm;
    m_wb.cond = ex_m_int.cond;
    m_wb.alu_output = ex_m_int.alu_output;
    m_wb.lmd = lmd;

    lmd = 0;
    ex_m_int = {};
}
