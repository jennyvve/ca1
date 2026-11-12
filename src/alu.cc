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
    int64_t signed_a = static_cast<int64_t>(A);
    int64_t signed_b = static_cast<int64_t>(B);
    switch (op) {
        case ALUOP::ADD:
            return static_cast<RegValue>(signed_a + signed_b);
        case ALUOP::SUB:
            return static_cast<RegValue>(signed_a - signed_b);
        case ALUOP::SLL: {
            unsigned shamt = static_cast<unsigned>(B & 0x3f);
            return A << shamt;
        }
        case ALUOP::SRL: {
            unsigned shamt = static_cast<unsigned>(B & 0x3f);
            return A >> shamt;
        }
        case ALUOP::SRA: {
            unsigned shamt = static_cast<unsigned>(B & 0x3f);
            RegValue result = A >> shamt;
            if (shamt != 0 && (A & (RegValue{1} << 63))) {
                result |= (~RegValue{0}) << (64 - shamt);
            }
            return result;
        }
        case ALUOP::SLT:
            return signed_a < signed_b;
        case ALUOP::SLTU:
            return A < B;
        case ALUOP::XOR:
            return A ^ B;
        case ALUOP::OR:
            return A | B;
        case ALUOP::AND:
            return A & B;
        case ALUOP::BEQ:
            return signed_a == signed_b;
        case ALUOP::BNE:
            return signed_a != signed_b;
        case ALUOP::BLT:
            return signed_a < signed_b;
        case ALUOP::BGE:
            return signed_a >= signed_b;
        case ALUOP::BLTU:
            return A < B;
        case ALUOP::BGEU:
            return A >= B;
        case ALUOP::ADD_32:
            return static_cast<RegValue>(static_cast<int32_t>(A) + static_cast<int32_t>(B));
        case ALUOP::SUB_32:
            return static_cast<RegValue>(static_cast<int32_t>(A) - static_cast<int32_t>(B));
        case ALUOP::SLL_32: {
            const uint32_t value = static_cast<uint32_t>(A);
            const unsigned shamt = static_cast<unsigned>(B & 0x1f);
            return static_cast<RegValue>(value << shamt);
        }
        case ALUOP::SRL_32: {
            const uint32_t value = static_cast<uint32_t>(A);
            const unsigned shamt = static_cast<unsigned>(B & 0x1f);
            return static_cast<RegValue>(value >> shamt);
        }
        case ALUOP::SRA_32: {
            const uint32_t value = static_cast<uint32_t>(A);
            const unsigned shamt = static_cast<unsigned>(B & 0x1f);
            uint32_t result = value >> shamt;
            if (shamt != 0 && (value & 0x80000000u)) {
                result |= (~uint32_t{0}) << (32 - shamt);
            }
            RegValue extended = result;
            if (result & 0x80000000u) {
                extended |= 0xffffffff00000000ULL;
            }
            return extended;
        }
        default:
            throw IllegalInstruction("Unimplemented or unknown ALU operation");
    }
}