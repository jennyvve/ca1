/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    fetch.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include <iostream>

#include "../stages.h"

void InstructionFetchStage::propagate() {
    mux_pc.set_input(InputSelectorPC::STAY, prog_counter);
    mux_pc.set_input(InputSelectorPC::DECREMENT, prog_counter - INSTR_SIZE);
    mux_pc.set_input(InputSelectorPC::INCREMENT, prog_counter + INSTR_SIZE);
    mux_flush.set_input(InputSelectorFlush::YES, true);
    mux_flush.set_input(InputSelectorFlush::NO, false);
    mux_flush.set_selector(InputSelectorFlush::NO);

    try {
        prog_counter = mux_pc.get_output();
        mux_pc.set_selector(InputSelectorPC::INCREMENT);

        instr_mem.setSize(INSTR_SIZE);
        instr_mem.setAddress(prog_counter);
        instr_reg = instr_mem.getValue();

        if (instr_reg != TestEndMarker) {
            instr_distributed = N_PIPELINE_STAGES;
        } else if (instr_distributed > 0) {
            instr_distributed--;
            mux_pc.set_selector(InputSelectorPC::STAY);
        }

        if (instr_reg == TestEndMarker &&
            (!pipelining || instr_distributed <= 0)) {
            throw TestEndMarkerEncountered(prog_counter);
        }
    } catch (TestEndMarkerEncountered &e) {
        throw;
    } catch (std::exception &e) {
        throw InstructionFetchFailure(prog_counter);
    }
}

void InstructionFetchStage::clock_pulse() {
    if (pipelining && mux_flush.get_output() == true) {
        if_id.flush();
    } else {
        if_id.npc = prog_counter;
        if_id.ir = instr_reg;
    }
}
