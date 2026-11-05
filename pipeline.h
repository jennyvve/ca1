/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    pipeline.h - Classic 5-stage RISC pipeline
 *
 * Copyright (C) 2016-2021  Leiden University, The Netherlands.
 */

#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "memory-control.h"
#include "stages.h"

class Pipeline {
   public:
    Pipeline(bool pipelining, bool debug_mode,
             MemAddress &prog_counter, InstructionMemory &instr_mem,
             InstructionDecoder &decoder, RegisterFile &regfile,
             DataMemory &data_memory);

    Pipeline(const Pipeline &) = delete;
    Pipeline &operator=(const Pipeline &) = delete;

    void propagate();
    void clock_pulse();

    bool getPipelining() const { return pipelining; }

    uint64_t getInstrIssued() const { return n_instr_issued; }

    uint64_t getInstrCompleted() const { return n_instr_completed; }

    uint64_t getStalls() const { return n_stalls; }

   private:
    bool pipelining;
    size_t currentStage{};

    /* Statistics */
    uint64_t n_instr_issued{};
    uint64_t n_instr_completed{};
    uint64_t n_stalls{};

    /* Stages */
    std::vector<std::unique_ptr<Stage>> stages{};

    /* Pipeline registers */
    IF_IDRegisters if_id{};
    ID_EXRegisters id_ex{};
    EX_MRegisters ex_m{};
    M_WBRegisters m_wb{};
};

#endif /* __PIPELINE_H__ */
