/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    alu.h - ALU component.
 *
 * Copyright (C) 2016  Leiden University, The Netherlands.
 */

#ifndef __ALU_H__
#define __ALU_H__

#include <map>

#include "arch.h"
#include "inst-decoder.h"

enum class ALUOP {
    ADD,
    NOP,
    SUB,
    SLL,
    SLT,
    SLT_32,
    SLTU,
    XOR,
    SRL,
    SRA,
    SRA_32,
    OR,
    AND
};

/* The ALU component performs the specified operation on operands A
 * and B when asked to propagate the result. The operation is
 * specified through the ALUOP.
 */
class ALU {
   public:
    ALU();

    void set_a(RegValue A) { this->A = A; }
    void set_b(RegValue B) { this->B = B; }

    RegValue getResult();

    void set_op(ALUOP op) { this->op = op; }

   private:
    RegValue A;
    RegValue B;

    ALUOP op;
};

#endif /* __ALU_H__ */
