/**
 *******************************************************************************
 * @file    pttk91.h
 * @author  Olli Vanhoja
 * @brief   PTT91 definitions
 *******************************************************************************
 */

/* PTTK91 instruction word:
 * +--------------------------------------------------------------------+
 * ¦  Operation code  ¦  Rj ¦ M ¦  Ri ¦      address part ADDR          ¦
 * ¦  8 bits          ¦  3  ¦ 2 ¦  3  ¦         16 bits                 ¦
 * +--------------------------------------------------------------------+
 * 31               24 23           16 15                               0
 */

#ifndef PTTK91_H
#define PTTK91_H

/* Instruction word definitions */
#define PTTK91_NUM_REGS     4
#define PTTK91_OPCODE_POS   24
#define PTTK91_RJ_POS       21
#define PTTK91_M_POS        19
#define PTTK91_PI_POS       16

/* Begin of PTTK91 opcodes */
#define PTTK91_NOP      0x0 << PTTK91_OPCODE_POS
#define PTTK91_STORE    0x1 << PTTK91_OPCODE_POS
#define PTTK91_LOAD     0x2 << PTTK91_OPCODE_POS
#define PTTK91_IN       0x3 << PTTK91_OPCODE_POS
#define PTTK91_OUT      0x4 << PTTK91_OPCODE_POS

#define PTTK91_ADD      0x11 << PTTK91_OPCODE_POS
#define PTTK91_SUB      0x12 << PTTK91_OPCODE_POS
#define PTTK91_MUL      0x13 << PTTK91_OPCODE_POS
#define PTTK91_DIV      0x14 << PTTK91_OPCODE_POS
#define PTTK91_MOD      0x15 << PTTK91_OPCODE_POS

#define PTTK91_AND      0x16 << PTTK91_OPCODE_POS
#define PTTK91_OR       0x17 << PTTK91_OPCODE_POS
#define PTTK91_XOR      0x18 << PTTK91_OPCODE_POS
#define PTTK91_SHL      0x19 << PTTK91_OPCODE_POS
#define PTTK91_SHR      0x1a << PTTK91_OPCODE_POS
#define PTTK91_NOT      0x1b << PTTK91_OPCODE_POS
#define PTTK91_SHRA     0x1c << PTTK91_OPCODE_POS

#define PTTK91_COMP     0x1f << PTTK91_OPCODE_POS

#define PTTK91_JUMP     0x20 << PTTK91_OPCODE_POS
#define PTTK91_JNEG     0x21 << PTTK91_OPCODE_POS
#define PTTK91_JZER     0x22 << PTTK91_OPCODE_POS
#define PTTK91_JPOS     0x23 << PTTK91_OPCODE_POS
#define PTTK91_JNNEG    0x24 << PTTK91_OPCODE_POS
#define PTTK91_JNZER    0x25 << PTTK91_OPCODE_POS
#define PTTK91_JNPOS    0x26 << PTTK91_OPCODE_POS

#define PTTK91_JLES     0x27 << PTTK91_OPCODE_POS
#define PTTK91_JEQU     0x28 << PTTK91_OPCODE_POS
#define PTTK91_JGRE     0x29 << PTTK91_OPCODE_POS
#define PTTK91_JNLES    0x2a << PTTK91_OPCODE_POS
#define PTTK91_JNEQU    0x2b << PTTK91_OPCODE_POS
#define PTTK91_JNGRE    0x2c << PTTK91_OPCODE_POS

#define PTTK91_CALL     0x31 << PTTK91_OPCODE_POS
#define PTTK91_EXIT     0x32 << PTTK91_OPCODE_POS
#define PTTK91_PUSH     0x33 << PTTK91_OPCODE_POS
#define PTTK91_POP      0x34 << PTTK91_OPCODE_POS
#define PTTK91_PUSHR    0x35 << PTTK91_OPCODE_POS
#define PTTK91_POPR     0x36 << PTTK91_OPCODE_POS

#define PTTK91_SVC      0x70 << PTTK91_OPCODE_POS
/* End of PTTK91 opcodes */

#endif /* PTTK91_H */
