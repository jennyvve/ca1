/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    pipeline.cc - Classic 5-stage RISC pipeline
 *
 * Copyright (C) 2016-2021  Leiden University, The Netherlands.
 */

#include "pipeline.h"

Pipeline::Pipeline(bool pipelining, bool debug_mode,
                   MemAddress &prog_counter,
                   InstructionMemory &instr_mem,
                   InstructionDecoder &decoder, RegisterFile &regfile,
                   DataMemory &data_memory)
    : pipelining{pipelining} {
    stages.emplace_back(std::make_unique<InstructionFetchStage>(
        pipelining, if_id, instr_mem, prog_counter));
    stages.emplace_back(std::make_unique<InstructionDecodeStage>(
        pipelining, if_id, id_ex, regfile, decoder, n_instr_issued,
        n_stalls, debug_mode));
    stages.emplace_back(
        std::make_unique<ExecuteStage>(pipelining, id_ex, ex_m));
    stages.emplace_back(std::make_unique<MemoryStage>(
        pipelining, ex_m, m_wb, data_memory));
    stages.emplace_back(std::make_unique<WriteBackStage>(
        pipelining, m_wb, regfile, n_instr_completed));
}

void Pipeline::propagate() {
    if (!pipelining) {
        /* Execute a single instruction execution step. */
        stages[currentStage]->propagate();
    } else {
        /* Run propagate for all stages within a single clock cycle.
         */
        for (auto &s : stages) s->propagate();
    }
}

void Pipeline::clock_pulse() {
    if (!pipelining) {
        stages[currentStage]->clock_pulse();
        currentStage = (currentStage + 1) % stages.size();
    } else {
        for (auto &s : stages) s->clock_pulse();
    }
}
