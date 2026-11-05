/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    stages.h - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#ifndef __STAGES_H__
#define __STAGES_H__

#include "alu.h"
#include "inst-decoder.h"
#include "memory-control.h"
#include "mux.h"

/* Pipeline registers may be read during propagate and may only be
 * written during clock_pulse. Note that you cannot read the incoming
 * pipeline registers in clock_pulse (e.g. in clock_pulse of EX, you
 * cannot read ID_EX) because that register will already have been
 * overwritten. In case you need to propagate values from one pipeline
 * register to the next, these need to be buffered explicitly within
 * the stage.
 */
struct IF_IDRegisters {
    MemAddress npc;
    RegValue ir;
};

struct ID_EXRegisters {
    MemAddress npc;
    instruction_t ir;
    RegValue a;
    RegValue b;
    RegValue imm;
};

struct EX_MRegisters {
    MemAddress npc;
    instruction_t ir;
    RegValue a;
    RegValue b;
    RegValue imm;
    RegValue cond;
    RegValue alu_output;
};

struct M_WBRegisters {
    MemAddress npc;
    instruction_t ir;
    RegValue a;
    RegValue b;
    RegValue imm;
    RegValue cond;
    RegValue alu_output;
    RegValue lmd;
};

/*
 * Abstract base class for pipeline stage
 */

class Stage {
   public:
    Stage(bool pipelining) : pipelining(pipelining) {}

    virtual ~Stage() {}

    virtual void propagate() = 0;
    virtual void clock_pulse() = 0;

   protected:
    bool pipelining;
};

/*
 * Instruction fetch
 */

class InstructionFetchFailure : public std::exception {
   public:
    explicit InstructionFetchFailure(const MemAddress addr) {
        std::stringstream ss;
        ss << "Instruction fetch failed at address " << std::hex
           << addr;
        message = ss.str();
    }

    const char *what() const noexcept override {
        return message.c_str();
    }

   private:
    std::string message{};
};

class TestEndMarkerEncountered : public std::exception {
   public:
    explicit TestEndMarkerEncountered(const MemAddress addr) {
        std::stringstream ss;
        ss << "Test end marker encountered at address " << std::hex
           << addr;
        message = ss.str();
    }

    const char *what() const noexcept override {
        return message.c_str();
    }

   private:
    std::string message{};
};

class InstructionFetchStage : public Stage {
   public:
    InstructionFetchStage(bool pipelining, IF_IDRegisters &if_id,
                          InstructionMemory instr_mem,
                          MemAddress &prog_counter)
        : Stage(pipelining),
          if_id(if_id),
          instr_mem(instr_mem),
          prog_counter(prog_counter) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    IF_IDRegisters &if_id;

    InstructionMemory instr_mem;
    RegValue instr_reg = 0;
    MemAddress &prog_counter;
};

/*
 * Instruction decode
 */

class InstructionDecodeStage : public Stage {
   public:
    InstructionDecodeStage(
        bool pipelining, const IF_IDRegisters &if_id,
        ID_EXRegisters &id_ex, RegisterFile &regfile,
        InstructionDecoder &decoder, uint64_t &n_instr_issued,
        uint64_t &n_stalls, bool debug_mode = false)
        : Stage(pipelining),
          if_id(if_id),
          id_ex(id_ex),
          regfile(regfile),
          decoder(decoder),
          n_instr_issued(n_instr_issued),
          n_stalls(n_stalls),
          debug_mode(debug_mode) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    const IF_IDRegisters &if_id;
    ID_EXRegisters &id_ex;

    RegisterFile &regfile;
    InstructionDecoder &decoder;

    uint64_t &n_instr_issued;
    uint64_t &n_stalls;

    instruction_t instr = {};
    RegValue a = 0;
    RegValue b = 0;
    RegValue imm = 0;

    bool debug_mode;
    /* TODO: add other necessary fields/buffers. */
};

/*
 * Execute
 */

class ExecuteStage : public Stage {
   public:
    ExecuteStage(bool pipelining, const ID_EXRegisters &id_ex,
                 EX_MRegisters &ex_m)
        : Stage(pipelining), id_ex(id_ex), ex_m(ex_m) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    const ID_EXRegisters &id_ex;
    EX_MRegisters &ex_m;

    void exec_handler_op(void);
    void exec_handler_load(void);
    void exec_handler_branch(void);

    typedef void (ExecuteStage::*exec_handler)(void);
    std::map<instruction_opcode_e, exec_handler> exec_map = {
        {INSTRUCTION_OPCODE_OP, &ExecuteStage::exec_handler_op},
        {INSTRUCTION_OPCODE_LOAD, &ExecuteStage::exec_handler_load},
        {INSTRUCTION_OPCODE_BRANCH,
         &ExecuteStage::exec_handler_branch},
    };

    ALU alu = ALU();
    RegValue alu_output = 0;
    RegValue cond = 0;
};

/*
 * Memory
 */

class MemoryStage : public Stage {
   public:
    MemoryStage(bool pipelining, const EX_MRegisters &ex_m,
                M_WBRegisters &m_wb, DataMemory data_memory)
        : Stage(pipelining),
          ex_m(ex_m),
          m_wb(m_wb),
          data_memory(data_memory) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    const EX_MRegisters &ex_m;
    M_WBRegisters &m_wb;

    void mem_handler_branch(void);
    void mem_handler_load(void);
    void mem_handler_store(void);

    typedef void (MemoryStage::*mem_handler)(void);
    std::map<instruction_opcode_e, mem_handler> mem_map = {
        {INSTRUCTION_OPCODE_BRANCH, &MemoryStage::mem_handler_branch},
        {INSTRUCTION_OPCODE_LOAD, &MemoryStage::mem_handler_load},
        {INSTRUCTION_OPCODE_STORE, &MemoryStage::mem_handler_store},
    };

    DataMemory data_memory;
    RegValue lmd = 0;
    /* TODO: add other necessary fields/buffers */
};

/*
 * Write back
 */

class WriteBackStage : public Stage {
   public:
    WriteBackStage(bool pipelining, const M_WBRegisters &m_wb,
                   RegisterFile &regfile, uint64_t &n_instr_completed)
        : Stage(pipelining),
          m_wb(m_wb),
          regfile(regfile),
          n_instr_completed(n_instr_completed) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    void wb_handler_op(void);
    void wb_handler_load(void);

    typedef void (WriteBackStage::*wb_handler)(void);
    std::map<instruction_opcode_e, wb_handler> wb_map = {
        {INSTRUCTION_OPCODE_OP, &WriteBackStage::wb_handler_op},
        {INSTRUCTION_OPCODE_LOAD, &WriteBackStage::wb_handler_load},
    };

    const M_WBRegisters &m_wb;

    RegisterFile &regfile;

    /* TODO add other necessary fields/buffers and components */

    uint64_t &n_instr_completed;
};

#endif /* __STAGES_H__ */
