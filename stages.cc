/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    stages.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include "stages.h"

#include <iostream>

enum class InputSelectorPC { INCREMENT, BRANCH, LAST };
enum class InputSelectorALUA { NPC, REGA, LAST };
enum class InputSelectorALUB { REGB, IMM, LAST };
enum class InputSelectorWB { MEM, REG, LAST };

Mux<RegValue, InputSelectorPC> mux_pc(InputSelectorPC::INCREMENT);

const uint8_t instr_size = 4;

void InstructionFetchStage::propagate() {
    try {
        instr_mem.setSize(instr_size);
        instr_mem.setAddress(prog_counter);
        instr_reg = instr_mem.getValue();

        mux_pc.set_input(InputSelectorPC::INCREMENT, prog_counter + instr_size);

        if (instr_reg == TestEndMarker) {
            throw TestEndMarkerEncountered(prog_counter);
        }
    } catch (TestEndMarkerEncountered &e) {
        throw;
    } catch (std::exception &e) {
        throw InstructionFetchFailure(prog_counter);
    }
}

void InstructionFetchStage::clock_pulse() {
    prog_counter = mux_pc.getOutput();
    if_id.npc = mux_pc.getOutput();
    if_id.ir = instr_reg;

    mux_pc.set_selector(InputSelectorPC::INCREMENT);
}

void dump_instruction(std::ostream &os, const uint32_t instr_reg, const InstructionDecoder &decoder);

void InstructionDecodeStage::propagate() {
    instr = decoder.set_instr_reg(if_id.ir);

    regfile.setRS1(instr.rs1);
    regfile.setRS2(instr.rs2);

    a = regfile.getReadData1();
    b = regfile.getReadData2();
    imm = instr.imm;

    if (debug_mode && (!pipelining || (pipelining && if_id.npc != 0x0))) {
        auto storeFlags(std::cerr.flags());
        std::cerr << std::hex << std::showbase << if_id.npc << "\t";
        std::cerr.setf(storeFlags);
        std::cerr << decoder << std::endl;
    }
}

void InstructionDecodeStage::clock_pulse() {
    if (!pipelining || (pipelining && if_id.npc != 0x0)) {
        ++n_instr_issued;
    }

    id_ex.npc = if_id.npc;
    id_ex.ir = instr;
    id_ex.a = a;
    id_ex.b = b;
    id_ex.imm = imm;
}

static std::map<std::pair<instruction_funct3_e, instruction_funct7_e>, ALUOP> map_instr_aluop = {
    {{INSTRUCTION_FUNCT3_OP_ADD_SUB, INSTRUCTION_FUNCT7_OP_ADD}, ALUOP::ADD},
    {{INSTRUCTION_FUNCT3_OP_SLL, INSTRUCTION_FUNCT7_OP_SLL}, ALUOP::SLL},
    {{INSTRUCTION_FUNCT3_OP_SLT, INSTRUCTION_FUNCT7_OP_SLT}, ALUOP::SLT},
    {{INSTRUCTION_FUNCT3_OP_SLTU, INSTRUCTION_FUNCT7_OP_SLTU}, ALUOP::SLTU},
    {{INSTRUCTION_FUNCT3_OP_XOR, INSTRUCTION_FUNCT7_OP_XOR}, ALUOP::XOR},
    {{INSTRUCTION_FUNCT3_OP_SRL_SRA, INSTRUCTION_FUNCT7_OP_SRL}, ALUOP::SRL},
    {{INSTRUCTION_FUNCT3_OP_SRL_SRA, INSTRUCTION_FUNCT7_OP_SRA}, ALUOP::SRA},
    {{INSTRUCTION_FUNCT3_OP_OR, INSTRUCTION_FUNCT7_OP_OR}, ALUOP::OR},
    {{INSTRUCTION_FUNCT3_OP_AND, INSTRUCTION_FUNCT7_OP_AND}, ALUOP::AND},
};

void ExecuteStage::exec_handler_op(void) {
    ALUOP op = map_instr_aluop[{id_ex.ir.funct3, id_ex.ir.funct7}];
    alu.set_a(id_ex.a);
    alu.set_b(id_ex.b);
    alu.set_op(op);
    alu_output = alu.getResult();
}

void ExecuteStage::exec_handler_load(void) { alu_output = id_ex.a + id_ex.imm; }

void ExecuteStage::exec_handler_branch(void) {
    alu_output = id_ex.npc + (id_ex.imm << 2);
    cond = (id_ex.a == id_ex.b);
}

void ExecuteStage::propagate() {
    if (exec_map.find(id_ex.ir.opcode) != exec_map.end()) {
        (this->*exec_map[id_ex.ir.opcode])();
    }
}

void ExecuteStage::clock_pulse() {
    ex_m.npc = id_ex.npc;
    ex_m.ir = id_ex.ir;
    ex_m.a = id_ex.a;
    ex_m.b = id_ex.b;
    ex_m.imm = id_ex.imm;
    ex_m.cond = cond;
    ex_m.alu_output = alu_output;
}

void MemoryStage::mem_handler_branch(void) {
    if (ex_m.cond) {
        mux_pc.set_selector(InputSelectorPC::BRANCH);
    }
}

std::map<instruction_funct3_e, bool> map_mem_sign_ext = {
    {INSTRUCTION_FUNCT3_LOAD_LB, true},  {INSTRUCTION_FUNCT3_LOAD_LH, true},   {INSTRUCTION_FUNCT3_LOAD_LW, true},
    {INSTRUCTION_FUNCT3_LOAD_LD, false}, {INSTRUCTION_FUNCT3_LOAD_LBU, false}, {INSTRUCTION_FUNCT3_LOAD_LHU, false},
};

void MemoryStage::mem_handler_load(void) {
    data_memory.setSize(1 << ex_m.ir.funct3);
    data_memory.setAddress(ex_m.alu_output);
    data_memory.setReadEnable(true);
    lmd = data_memory.getDataOut(map_mem_sign_ext[ex_m.ir.funct3]);
    std::cout << lmd << std::endl;
}

void MemoryStage::mem_handler_store(void) {
    data_memory.setSize(1 << ex_m.ir.funct3);
    data_memory.setAddress(ex_m.alu_output);
    data_memory.setDataIn(ex_m.b);
    data_memory.setWriteEnable(true);
}

void MemoryStage::propagate() {
    mux_pc.set_input(InputSelectorPC::BRANCH, ex_m.alu_output);

    if (mem_map.find(ex_m.ir.opcode) != mem_map.end()) {
        (this->*mem_map[ex_m.ir.opcode])();
    }
}

void MemoryStage::clock_pulse() {
    data_memory.clock_pulse();
    data_memory.setReadEnable(false);
    data_memory.setWriteEnable(false);

    m_wb.npc = ex_m.npc;
    m_wb.ir = ex_m.ir;
    m_wb.a = ex_m.a;
    m_wb.b = ex_m.b;
    m_wb.imm = ex_m.imm;
    m_wb.cond = ex_m.cond;
    m_wb.alu_output = ex_m.alu_output;
    m_wb.lmd = lmd;
    lmd = 0;
}

void WriteBackStage::wb_handler_op(void) {
    regfile.setRD(m_wb.ir.rd);
    regfile.setWriteData(m_wb.alu_output);
    regfile.setWriteEnable(true);
    regfile.clock_pulse();
    regfile.setWriteEnable(false);
}

void WriteBackStage::wb_handler_load(void) {
    regfile.setRD(m_wb.ir.rd);
    regfile.setWriteEnable(true);
    regfile.setWriteData(m_wb.lmd);
    regfile.setWriteEnable(false);
    regfile.clock_pulse();
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
