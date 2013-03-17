/**
 ******************************************************************************
 * @file    vm.h
 * @author  Olli Vanhoja
 * @brief   This file implements the core of PTTK91 virtual machine.
 ******************************************************************************
 */

#ifndef VM_H
#define VM_H

#include "pttk91.h"

/**
 * Virtual machine state.
 */
struct vm_state {
    int regs[PTTK91_NUM_REGS];
    int pc; /* program counter */

    /* Rest of variables for internal use */
    int opcode;
    /* operands: */
    int rj;
    int m;
    int ri;
    int imm;

    /**
     * state register.<br/>
     * <b>index:</b>
     * + gre - greater
     * + equ - equal
     * + les - less
     * + ovf - arithmetic overflow
     * + div - divide by zero
     * + uni - unknown instruction
     * + fma - forbidden memory access
     * + dei - device interrupt
     * + svc - supervisor call
     * + pri - priviledged mode
     * + nin - interrupts disabled
     */
    struct sr_t {
        unsigned int gre : 1;
        unsigned int equ : 1;
        unsigned int les : 1;
        unsigned int ovf : 1;
        unsigned int div : 1;
        unsigned int uni : 1;
        unsigned int fma : 1;
        unsigned int dei : 1;
        unsigned int svc : 1;
        unsigned int pri : 1;
        unsigned int nin : 1;
    } sr;

    /* the VM runs until this flag becomes 0 */
    int running;
};

void init_vm_state(struct vm_state * state);
void run(struct vm_state * state, uint32_t * mem, int memsize);

#endif /* VM_H */
