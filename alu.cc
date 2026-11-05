/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    alu.h - ALU component.
 *
 * Copyright (C) 2016,2018  Leiden University, The Netherlands.
 */

#include "alu.h"

#include "inst-decoder.h"

#ifdef _MSC_VER
/* MSVC intrinsics */
#include <intrin.h>
#endif

ALU::ALU() : A(), B(), op() {}

RegValue ALU::getResult() {
    const uint32_t shift_mask = (sizeof(RegValue) == 8) ? 0x3F : 0x1F;

    switch (op) {
        case ALUOP::ADD:
            return A + B;
        case ALUOP::SUB:
            return A - B;
        case ALUOP::SLL:
            return A << (B & shift_mask);
        case ALUOP::SRL:
            return A >> (B & shift_mask);
        case ALUOP::SRA: {
            int64_t signed_a = static_cast<int64_t>(A);
            return static_cast<RegValue>(signed_a >>
                                         (B & shift_mask));
        }
        case ALUOP::SRA_32: {
            int32_t signed_a = static_cast<int32_t>(A);
            return static_cast<RegValue>(signed_a >>
                                         (B & shift_mask));
        }
        case ALUOP::SLT:
            return static_cast<int64_t>(A) < static_cast<int64_t>(B);
        case ALUOP::SLT_32:
            return static_cast<int32_t>(A) < static_cast<int32_t>(B);
        case ALUOP::SLTU:
            return A < B;
        case ALUOP::XOR:
            return A ^ B;
        case ALUOP::OR:
            return A | B;
        case ALUOP::AND:
            return A & B;
        default:
            throw IllegalInstruction(
                "Unimplemented or unknown ALU operation");
    }
}