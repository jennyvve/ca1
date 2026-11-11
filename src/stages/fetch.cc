/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    fetch.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include <iostream>

#include "../stages.h"

const uint8_t N_PIPELINE_STAGES = 5;
const uint8_t INSTR_SIZE = 4;

void InstructionFetchStage::propagate() {
    mux_pc.set_input(InputSelectorPC::INCREMENT, prog_counter);
    mux_flush.set_input(InputSelectorFlush::YES, true);
    mux_flush.set_input(InputSelectorFlush::NO, false);
    mux_flush.set_selector(InputSelectorFlush::NO);

    try {
        instr_mem.setSize(INSTR_SIZE);
        instr_mem.setAddress(mux_pc.get_output());
        instr_reg = instr_mem.getValue();

        if (instr_reg != TestEndMarker) {
            instr_distributed = N_PIPELINE_STAGES;
        } else if (instr_distributed > 0) {
            instr_distributed--;
            mux_pc.set_input(InputSelectorPC::INCREMENT, prog_counter - INSTR_SIZE);
        }

        if (instr_reg == TestEndMarker && instr_distributed <= 0) {
            throw TestEndMarkerEncountered(prog_counter);
        }
    } catch (TestEndMarkerEncountered &e) {
        throw;
    } catch (std::exception &e) {
        throw InstructionFetchFailure(prog_counter);
    }
}

void InstructionFetchStage::clock_pulse() {
    if (mux_flush.get_output() == true) {
        if_id.flush();
    } else {
        prog_counter = mux_pc.get_output() + INSTR_SIZE;
        if_id.npc = mux_pc.get_output();
        if_id.ir = instr_reg;
        mux_pc.set_selector(InputSelectorPC::INCREMENT);
    }
}
