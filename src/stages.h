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

typedef enum {
    DATAFORWARD_NONE = 0,
    DATAFORWARD_EX = 1,
    DATAFORWARD_MEM = 2,
} dataforward_map_e;

static instruction_t instr_nop = {
    .opcode = INSTRUCTION_OPCODE_OP_IMM,
    .rd = 0,
    .rs1 = 0,
    .imm = 0,
};

struct IF_IDRegisters {
    MemAddress npc;
    RegValue ir;

    void flush() {
        npc = 0;
        ir = 0;
    }
};

struct ID_EXRegisters {
    MemAddress npc;
    instruction_t ir{};
    RegValue a;
    RegValue b;
    RegValue imm;
    dataforward_map_e df_a;
    dataforward_map_e df_b;

    void flush() {
        npc = a = b = imm = 0;
        df_a = df_b = DATAFORWARD_NONE;
        ir = instr_nop;
    }
};

struct EX_MRegisters {
    MemAddress npc;
    instruction_t ir{};
    RegValue a;
    RegValue b;
    RegValue imm;
    RegValue cond;
    RegValue alu_output;

    void flush() {
        npc = a = b = imm = cond = alu_output = 0;
        ir = instr_nop;
    }
};

struct M_WBRegisters {
    MemAddress npc;
    instruction_t ir{};
    RegValue a;
    RegValue b;
    RegValue imm;
    RegValue cond;
    RegValue alu_output;
    RegValue lmd;

    void flush() {
        npc = 0;
        ir = instr_nop;
        a = 0;
        b = 0;
        imm = 0;
        cond = 0;
        alu_output = 0;
        lmd = 0;
    }
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
        ss << "Instruction fetch failed at address " << std::hex << addr;
        message = ss.str();
    }

    const char *what() const noexcept override { return message.c_str(); }

   private:
    std::string message{};
};

class TestEndMarkerEncountered : public std::exception {
   public:
    explicit TestEndMarkerEncountered(const MemAddress addr) {
        std::stringstream ss;
        ss << "Test end marker encountered at address " << std::hex << addr;
        message = ss.str();
    }

    const char *what() const noexcept override { return message.c_str(); }

   private:
    std::string message{};
};

class InstructionFetchStage : public Stage {
   public:
    InstructionFetchStage(bool pipelining, IF_IDRegisters &if_id, InstructionMemory instr_mem, MemAddress &prog_counter)
        : Stage(pipelining), if_id(if_id), instr_mem(instr_mem), prog_counter(prog_counter) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    IF_IDRegisters &if_id;

    InstructionMemory instr_mem;
    RegValue instr_reg = 0;
    MemAddress &prog_counter;
    int8_t instr_distributed = 0;
};

/*
 * Instruction decode
 */

class InstructionDecodeStage : public Stage {
   public:
    InstructionDecodeStage(bool pipelining, const IF_IDRegisters &if_id, ID_EXRegisters &id_ex, RegisterFile &regfile,
                           InstructionDecoder &decoder, uint64_t &n_instr_issued, uint64_t &n_stalls,
                           bool debug_mode = false)
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
    RegValue imm = 0;
    dataforward_map_e df_a = DATAFORWARD_NONE;
    dataforward_map_e df_b = DATAFORWARD_NONE;

    uint32_t reg_rd_prev[2] = {};

    bool debug_mode;
    IF_IDRegisters if_id_int{};
};

/*
 * Execute
 */

class ExecuteStage : public Stage {
   public:
    ExecuteStage(bool pipelining, const ID_EXRegisters &id_ex, EX_MRegisters &ex_m)
        : Stage(pipelining), id_ex(id_ex), ex_m(ex_m) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    const ID_EXRegisters &id_ex;
    EX_MRegisters &ex_m;

    void exec_handler_lui(void);
    void exec_handler_op_imm(void);
    void exec_handler_op_imm_32(void);
    void exec_handler_op(void);
    void exec_handler_op_32(void);
    void exec_handler_mem(void);
    void exec_handler_branch(void);

    typedef void (ExecuteStage::*exec_handler)(void);
    std::map<instruction_opcode_e, exec_handler> exec_map = {
        {INSTRUCTION_OPCODE_LUI, &ExecuteStage::exec_handler_lui},
        {INSTRUCTION_OPCODE_OP_IMM_32, &ExecuteStage::exec_handler_op_imm_32},
        {INSTRUCTION_OPCODE_OP_IMM, &ExecuteStage::exec_handler_op_imm},
        {INSTRUCTION_OPCODE_OP, &ExecuteStage::exec_handler_op},
        {INSTRUCTION_OPCODE_OP_32, &ExecuteStage::exec_handler_op_32},
        {INSTRUCTION_OPCODE_LOAD, &ExecuteStage::exec_handler_mem},
        {INSTRUCTION_OPCODE_STORE, &ExecuteStage::exec_handler_mem},
        {INSTRUCTION_OPCODE_BRANCH, &ExecuteStage::exec_handler_branch},
    };

    ALU alu = ALU();
    RegValue alu_output = 0;
    RegValue cond = 0;
    ID_EXRegisters id_ex_int{};

    RegValue dataforward_pipeline[3] = {};
};

/*
 * Memory
 */

class MemoryStage : public Stage {
   public:
    MemoryStage(bool pipelining, const EX_MRegisters &ex_m, M_WBRegisters &m_wb, DataMemory data_memory)
        : Stage(pipelining), ex_m(ex_m), m_wb(m_wb), data_memory(data_memory) {}

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
    EX_MRegisters ex_m_int{};
};

/*
 * Write back
 */

class WriteBackStage : public Stage {
   public:
    WriteBackStage(bool pipelining, const M_WBRegisters &m_wb, RegisterFile &regfile, uint64_t &n_instr_completed)
        : Stage(pipelining), m_wb(m_wb), regfile(regfile), n_instr_completed(n_instr_completed) {}

    void propagate() override;
    void clock_pulse() override;

   private:
    void wb_handler_op(void);
    void wb_handler_load(void);
    void wb_handler_store(void);

    typedef void (WriteBackStage::*wb_handler)(void);
    std::map<instruction_opcode_e, wb_handler> wb_map = {
        {INSTRUCTION_OPCODE_LUI, &WriteBackStage::wb_handler_op},
        {INSTRUCTION_OPCODE_OP_IMM, &WriteBackStage::wb_handler_op},
        {INSTRUCTION_OPCODE_OP, &WriteBackStage::wb_handler_op},
        {INSTRUCTION_OPCODE_LOAD, &WriteBackStage::wb_handler_load},
    };

    const M_WBRegisters &m_wb;

    RegisterFile &regfile;

    uint64_t &n_instr_completed;
};

enum class InputSelectorPC { INCREMENT, BRANCH, LAST };
enum class InputSelectorFlush { NO, YES, LAST };

inline Mux<RegValue, InputSelectorPC> mux_pc(InputSelectorPC::INCREMENT);
inline Mux<bool, InputSelectorFlush> mux_flush(InputSelectorFlush::NO);

#endif /* __STAGES_H__ */
