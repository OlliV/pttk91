/**
 ******************************************************************************
 * @file    vm.c
 * @author  Olli Vanhoja
 * @brief   This file implements the core of PTTK91 virtual machine.
 ******************************************************************************
 */

/** @addtogroup VM
  * @{
  */

/** TODO Interrupt vector, interrupts & Timer */

#include <stdio.h>
#include <stdint.h>

#include "arit.h"
#include "pttk91.h"

/* Error codes */
#define VM_ERR_INVALID_OPCODE           -1
#define VM_ERR_ADDRESS_OUT_OF_BOUNDS    -2
#define VM_ERR_REGISTER_OUT_OF_BOUNDS   -3
#define VM_ERR_BAD_ACCESS_MODE          -4

/* Macros */
#define VM_REG_OUT_OF_BOUNDS(regX)              (regX < 0 || regX >= PTTK91_NUM_REGS)
#define VM_MEM_OUT_OF_BOUNDS(memaddr, memsize)  (memaddr >= memsize || memaddr < 0)

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

/**
 * Initializes a vm_state structure.
 */
void init_vm_state(struct vm_state * state)
{
    int i;

    for (i = 0; i < PTTK91_NUM_REGS; i++) {
        state->regs[i] = 0;
    }
    state->pc = 0;
    state->opcode = 0;
    state->rj = 0;
    state->m = 0;
    state->ri = 0;
    state->imm = 0;
    state->running = 1;
}

/**
 * Fetch next instruction.
 */
static int fetch(uint32_t * instr, struct vm_state * state, const uint32_t * mem, int memsize)
{
    int pc = state->pc;
    if (VM_MEM_OUT_OF_BOUNDS(pc, memsize)) {
        /* PC out of bounds */
        return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
    }

    *instr = mem[state->pc++];
    return 0;
}

/**
 *  Decode a word.
 */
static int decode(struct vm_state * state, uint32_t instr)
{
    state->opcode   = (instr & 0xFF000000);
    state->rj       = (instr & 0x00E00000) >> PTTK91_RJ_POS;
    state->m        = (instr & 0x00180000) >> PTTK91_M_POS;
    state->ri       = (instr & 0x00070000) >> PTTK91_PI_POS;
    state->imm      = (instr & 0x000fffff);

    if (VM_REG_OUT_OF_BOUNDS(state->rj) || VM_REG_OUT_OF_BOUNDS(state->ri)) {
        return VM_ERR_REGISTER_OUT_OF_BOUNDS;
    }

    return 0;
}

/**
 * Evaluate the last decoded instruction.
 */
static int eval(struct vm_state * state, uint32_t * mem, int memsize)
{
    int rj = state->rj;
    int ri = state->ri;
    int param;

    param = state->imm;
    if (ri != 0) {
        /* Add indexing register Ri */
        param += state->regs[ri];
    }
    if (state->m == 1) {
        /* direct memory fetch */
        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        param = mem[param];
    } else if (state->m == 2) {
        int opcode = state->opcode;
        if ((opcode >= PTTK91_JUMP && opcode <= PTTK91_JNGRE) || (opcode == PTTK91_STORE)) {
            /* + For all branching instructions: if m == 2 => bad access mode
             * + PTTK91_STORE if state->m == 2 => bad access mode
             */
            return VM_ERR_BAD_ACCESS_MODE;
        }

        /* Pointer fetch */
        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        param = mem[param];

        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        param = mem[param];
    } else if (state->m == 3) {
        /* There is no mode 3 */
        return VM_ERR_BAD_ACCESS_MODE;
    }

    /* Execute */
    switch(state->opcode) {
    case PTTK91_NOP:
        break;

    case PTTK91_STORE:
        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        mem[param] = state->regs[rj];
        break;
    case PTTK91_LOAD:
        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        state->regs[rj] = mem[param];
        break;
    case PTTK91_IN:
        /* TODO */
        break;
    case PTTK91_OUT:
        /* TODO portable version */
        if (param == 0) {
            printf("CRT output: %i\n", state->regs[rj]);
        }
        break;

    case PTTK91_ADD:
        state->regs[rj] = state->regs[rj] + param;
        break;
    case PTTK91_SUB:
        state->regs[rj] = state->regs[rj] - param;
        break;
    case PTTK91_MUL:
        state->regs[rj] = state->regs[rj] * param;
        break;
    case PTTK91_DIV:
        /* TODO div by zero check? */
        state->regs[rj] = state->regs[rj] / param;
        break;
    case PTTK91_MOD:
        /* TODO div by zero check? */
        state->regs[rj] = state->regs[rj] % param;
        break;

    case PTTK91_AND:
        state->regs[rj] = (unsigned int)(state->regs[rj]) & (unsigned int)param;
        break;
    case PTTK91_OR:
        state->regs[rj] = (unsigned int)(state->regs[rj]) | (unsigned int)param;
        break;
    case PTTK91_XOR:
        state->regs[rj] = (unsigned int)(state->regs[rj]) ^ (unsigned int)param;
        break;
    case PTTK91_SHL:
        state->regs[rj] = (unsigned int)(state->regs[rj]) << (unsigned int)param;
        break;
    case PTTK91_SHR:
        state->regs[rj] = (unsigned int)(state->regs[rj]) >> (unsigned int)param;
        break;
    case PTTK91_NOT:
        state->regs[rj] = ~((unsigned int)(state->regs[rj]));
        break;
    case PTTK91_SHRA:
        state->regs[rj] = arithmetic_right_shift(state->regs[rj], (unsigned int)param);
        break;

    case PTTK91_COMP:
        if (state->regs[rj] > param) {
            state->sr.gre = 1;
            state->sr.equ = 0;
            state->sr.les = 0;
        } else if (state->regs[rj] < param) {
            state->sr.gre = 0;
            state->sr.equ = 0;
            state->sr.les = 1;
        } else {
            state->sr.gre = 0;
            state->sr.equ = 1;
            state->sr.les = 0;
        }
        break;

    case PTTK91_JUMP:
        state->pc = param;
        break;
    case PTTK91_JNEG:
        if (state->regs[rj] < 0) {
            state->pc = param;
        }
        break;
    case PTTK91_JZER:
        if (state->regs[rj] == 0) {
            state->pc = param;
        }
        break;
    case PTTK91_JPOS:
        if (state->regs[rj] > 0) {
            state->pc = param;
        }
        break;
    case PTTK91_JNNEG:
        if (state->regs[rj] >= 0) {
            state->pc = param;
        }
        break;
    case PTTK91_JNZER:
        if (state->regs[rj] != 0) {
            state->pc = param;
        }
        break;
    case PTTK91_JNPOS:
        if (state->regs[rj] <= 0) {
            state->pc = param;
        }
        break;

    case PTTK91_JLES:
        if (state->sr.les) {
            state->pc = param;
        }
        break;
    case PTTK91_JEQU:
        if (state->sr.equ) {
            state->pc = param;
        }
        break;
    case PTTK91_JGRE:
        if (state->sr.gre) {
            state->pc = param;
        }
        break;
    case PTTK91_JNLES:
        if (state->sr.equ || state->sr.gre) {
            state->pc = param;
        }
        break;
    case PTTK91_JNEQU:
        if (state->sr.les || state->sr.gre) {
            state->pc = param;
        }
        break;
    case PTTK91_JNGRE:
        if (state->sr.les || state->sr.equ) {
            state->pc = param;
        }
        break;

    case PTTK91_CALL:
        /* TODO */
        break;
    case PTTK91_EXIT:
        /* TODO */
        break;
    case PTTK91_PUSH:
        /* TODO */
        break;
    case PTTK91_POP:
        /* TODO */
        break;
    case PTTK91_PUSHR:
        /* TODO */
        break;
    case PTTK91_POPR:
        /* TODO */
        break;

    case PTTK91_SVC:
        /* TODO */

        /* halt */
        printf("halt\n");
        state->running = 0;
        break;
    default:
        printf("Illegal instruction\n");
        break;
    }

    return 0;
}

/**
 * Print all registers.
 */
void showRegs(const struct vm_state * state)
{
    int i;
    printf("regs = ");
    for(i = 0; i < PTTK91_NUM_REGS; i++)
        printf( "r%i: %04X, ", i, state->regs[i]);
    printf("PC %i \n", state->pc);
}

void run(struct vm_state * state, uint32_t * mem, int memsize)
{
    uint32_t instr;

    do {
        showRegs(state);
        if(fetch(&instr, state, mem, memsize)) {
            /* TODO PC out of bounds */
            printf("PC out of bounds");
            return;
        }
        decode(state, instr);
        eval(state, mem, memsize);
    } while (state->running);
    showRegs(state);
 }

int main( int argc, const char * argv[] )
{
    uint32_t prog[] = { 0x1120000a, 0x04200000, 0x7000000b };
    int memsize = 3;
    struct vm_state state;

    init_vm_state(&state);
    run(&state, prog, memsize);
    return 0;
}

/**
  * @}
  */

