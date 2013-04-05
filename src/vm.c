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

#include <stdio.h>
#include <stdint.h>
#include "arit.h"
#include "inp.h"
#include "outp.h"
#include "svc.h"
#include "vm.h"

/* Error codes */
#define VM_ERR_NO_ERROR                 0
#define VM_ERR_INVALID_OPCODE           1
#define VM_ERR_PARAM_ERROR              2
#define VM_ERR_ADDRESS_OUT_OF_BOUNDS    3
#define VM_ERR_WR_ADDRESS_OUT_OF_BOUNDS 4
#define VM_ERR_REGISTER_OUT_OF_BOUNDS   5
#define VM_ERR_PC_OUT_OF_BOUNDS         6
#define VM_ERR_BAD_ACCESS_MODE          7
#define VM_ERR_ILLEGAL_SVC              8
#define VM_ERR_INVALID_DEVICE           9

/* Error message macros */
#define VM_ERR_STR(code)        case code: fprintf(stderr, "%i, %s\n", code, #code); return
#define VM_ERR_STR2(code, msg)  case code: return fprintf(stderr, "%i, %s\n", code, msg); return

/**
 * Print error message.
 * @param error_code indicates the error code of the message that should be printed.
 */
static void print_error_msg(int error_code)
{
    fprintf(stderr, "Runtime error: ");
    switch (error_code)
    {
        VM_ERR_STR(VM_ERR_NO_ERROR); /* Hope this pops up somewhere ^_^ */
        VM_ERR_STR(VM_ERR_INVALID_OPCODE);
        VM_ERR_STR(VM_ERR_PARAM_ERROR);
        VM_ERR_STR(VM_ERR_ADDRESS_OUT_OF_BOUNDS);
        VM_ERR_STR(VM_ERR_WR_ADDRESS_OUT_OF_BOUNDS);
        VM_ERR_STR(VM_ERR_REGISTER_OUT_OF_BOUNDS);
        VM_ERR_STR(VM_ERR_PC_OUT_OF_BOUNDS);
        VM_ERR_STR(VM_ERR_BAD_ACCESS_MODE);
        VM_ERR_STR(VM_ERR_ILLEGAL_SVC);
        VM_ERR_STR(VM_ERR_INVALID_DEVICE);
    }
}
#undef VM_ERR_STR
#undef VM_ERR_STR2 /* These should not be used anywhere else */

/* Macros */
/* Here is some macros for mainly bounds checking.
 */
#define VM_REG_OUT_OF_BOUNDS(regX)              (regX < 0 || regX >= PTTK91_NUM_REGS)
/* Check if mem address is between 0 and memsize */
#define VM_MEM_OUT_OF_BOUNDS(memaddr, memsize)  (memaddr >= memsize || memaddr < 0)

#if VM_DATA_ALLOW_PC == 0
#define VM_PC_OUT_OF_BOUNDS(pc, codesize, memsize) (pc > codesize || VM_MEM_OUT_OF_BOUNDS(pc, memsize))
#elif VM_DATA_ALLOW_PC == 1
#define VM_PC_OUT_OF_BOUNDS(pc, codesize, memsize) VM_MEM_OUT_OF_BOUNDS(pc, memsize)
#else
#error Incorrect value of VM_DATA_ALLOW_PC
#endif

#if VM_CODE_AREA_RW == 0
#define VM_MEM_OUT_OF_BOUNDS_STORE(memaddr, codesize, memsize)  (memaddr >= memsize || memaddr < codesize)
#elif VM_CODE_AREA_RW == 1
#define VM_MEM_OUT_OF_BOUNDS_STORE(memaddr, codesize, memsize)  (memaddr >= memsize || memaddr < 0)
#else
#error Incorrect value of VM_CODE_AREA_RW
#endif
/* End of Macros */

/**
 * Initializes a vm_state structure.
 * @param state vm state.
 * @param code_size size of code section.
 */
void vm_init_state(struct vm_state * state, int code_size, int memsize)
{
    int i;

    for (i = 0; i < PTTK91_NUM_REGS; i++) {
        state->regs[i] = 0;
    }
    state->regs[PTTK91_SP] = code_size - 1;
    state->regs[PTTK91_FP] = code_size - 1;

    state->pc = 0;
    state->opcode = 0;
    state->rj = 0;
    state->m = 0;
    state->ri = 0;
    state->imm = 0;

    /* Clear status register */
    state->sr.gre = 0;
    state->sr.equ = 0;
    state->sr.les = 0;
    state->sr.ovf = 0;
    state->sr.div = 0;
    state->sr.uni = 0;
    state->sr.fma = 0;
    state->sr.dei = 0;
    state->sr.svc = 0;
    state->sr.pri = 0;
    state->sr.nin = 0;

    state->memsize = memsize;
    state->code_sec_end = code_size;
    state->running = 1;
}

/**
 * Fetch next instruction.
 */
static int fetch(uint32_t * instr, struct vm_state * state, const uint32_t * mem)
{
    if (VM_PC_OUT_OF_BOUNDS(state->pc, state->code_sec_end, state->memsize)) {
        /* PC out of bounds */
        return VM_ERR_PC_OUT_OF_BOUNDS;
    }

    *instr = mem[state->pc++];
    return 0;
}

/**
 *  Decode a instruction word.
 */
static int decode(struct vm_state * state, uint32_t instr)
{
    state->opcode   = (int)(instr & 0xFF000000);
    state->rj       = (int)((instr & 0x00E00000) >> PTTK91_RJ_POS);
    state->m        = (int)(instr & 0x00180000);
    state->ri       = (int)((instr & 0x00070000) >> PTTK91_RI_POS);
    state->imm      = (int)(instr & 0x0000ffff);

    if (VM_REG_OUT_OF_BOUNDS(state->rj) || VM_REG_OUT_OF_BOUNDS(state->ri)) {
        return VM_ERR_REGISTER_OUT_OF_BOUNDS;
    }

    return 0;
}

/**
 * Evaluate the last decoded instruction.
 */
static int eval(struct vm_state * state, uint32_t * mem)
{
    /* Copy some data for (hopefully) faster access */
    int opcode = state->opcode;
    int rj = state->rj;
    int ri = state->ri;
    int memsize = state->memsize;

    int param; /* Final second arg value will be stored to this variable */
    int i, sp; /* Temp variables */

    param = state->imm; /* Starting point for "parsing" the final value */
    if (ri != 0) {
        /* Add indexing register Ri */
        param += state->regs[ri];
    }
    if (state->m == PTTK91_ADDRMOD_1) { /* Direct memory fetch */
        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        param = mem[param];
    } else if (state->m == PTTK91_ADDRMOD_2) { /* Indirect meory fetch */
        if ((opcode >= PTTK91_JUMP && opcode <= PTTK91_JNGRE)
            || (opcode == PTTK91_STORE)) {
            /* + For all branching instructions: mode 2 is bad access mode
             * + PTTK91_STORE doesn't support mode 2
             */
            return VM_ERR_BAD_ACCESS_MODE;
        }

        /* First fetch */
        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        param = mem[param];

        /* Second fetch */
        if (VM_MEM_OUT_OF_BOUNDS(param, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        param = mem[param];
    } else if (state->m == PTTK91_ADDRMOD_3) {
        /* Mode 3 is not specified */
        return VM_ERR_BAD_ACCESS_MODE;
    }

    /* Execute */
    switch(opcode) {
    case PTTK91_NOP:
        break;

    /* Data transfer instructions */
    case PTTK91_STORE:
        if (VM_MEM_OUT_OF_BOUNDS_STORE(param, state->code_sec_end, memsize)) {
            return VM_ERR_WR_ADDRESS_OUT_OF_BOUNDS;
        }
        mem[param] = state->regs[rj];
        break;
    case PTTK91_LOAD:
        state->regs[rj] = param;
        break;
    case PTTK91_IN:
        if (inp_handler(param, &(state->regs[rj]))) {
            return VM_ERR_INVALID_DEVICE;
        }
        break;
    case PTTK91_OUT:
        if (outp_handler(param, state->regs[rj])) {
            return VM_ERR_INVALID_DEVICE;
        }
        break;

    /* Arithmetic instructions */
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
        /* Div by zero check */
        if (param == 0) {
            state->sr.div = 1;
            return VM_ERR_PARAM_ERROR;
        }
        state->regs[rj] = state->regs[rj] / param;
        break;
    case PTTK91_MOD:
        /* Div by zero check */
        if (param == 0) {
            state->sr.div = 1;
            return VM_ERR_PARAM_ERROR;
        }
        state->regs[rj] = state->regs[rj] % param;
        break;

    /* Logic instructions */
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

    /* Branching instructions */
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

    /* Subroutine instructions */
    case PTTK91_CALL:
        state->regs[rj] = state->regs[rj] + 2;

        /* Check that the new stack pointer is valid */
        if (VM_MEM_OUT_OF_BOUNDS_STORE(state->regs[rj], state->code_sec_end, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }

        mem[state->regs[rj] - 1] = state->pc; /* Push PC */
        mem[state->regs[rj]] = state->regs[PTTK91_FP]; /* Push FP */
        state->regs[PTTK91_FP] = state->regs[rj]; /* Set new FP */
        state->pc = param; /* Branch */
        break;
    case PTTK91_EXIT:
        sp = state->regs[PTTK91_FP];

        /* Check that the old fp location is valid */
        if (VM_MEM_OUT_OF_BOUNDS(sp, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }

        /* Check that the new sp & fp are valid */
        if (VM_MEM_OUT_OF_BOUNDS(sp - 2 - param, memsize)
            || VM_MEM_OUT_OF_BOUNDS((int)(mem[sp]), memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }

        /* Read back the original sp & fp values */
        state->regs[rj] = sp - 2 - param;
        state->regs[PTTK91_FP] = mem[sp];
        state->pc = mem[sp - 1]; /* Return */
        break;

    /* Stack instructions */
    case PTTK91_PUSH:
        state->regs[rj] = state->regs[rj] + 1;
        sp = state->regs[rj];

        /* Check that the new sp value is valid */
        if (VM_MEM_OUT_OF_BOUNDS_STORE(sp, state->code_sec_end, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        mem[sp] = param;
        break;
    case PTTK91_POP:
        sp = state->regs[rj];
        /* POP: Second operand should be always a register */

        if (state->m != 0) {
            return VM_ERR_BAD_ACCESS_MODE;
        }

        if (VM_MEM_OUT_OF_BOUNDS_STORE(sp, state->code_sec_end, memsize)) {
            return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
        }
        state->regs[ri] = mem[sp];
        state->regs[rj] = sp - 1;
        break;
    case PTTK91_PUSHR: /* Push R0..R6 */
        sp = state->regs[rj] + 1;
        state->regs[rj] = state->regs[rj] + PTTK91_NUM_REGS - 1;

        for (i = 0; i < PTTK91_NUM_REGS - 1; i++) {
            if (VM_MEM_OUT_OF_BOUNDS_STORE(sp, state->code_sec_end, memsize)) {
                return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
            }
            mem[sp++] = state->regs[i];
        }
        break;
    case PTTK91_POPR: /* Pop R6..R0 */
        for (i = PTTK91_NUM_REGS - 2; i >= 0; i--) {
            if (VM_MEM_OUT_OF_BOUNDS_STORE(state->regs[rj], state->code_sec_end, memsize)) {
                return VM_ERR_ADDRESS_OUT_OF_BOUNDS;
            }
            state->regs[i] = mem[state->regs[rj - i]];
        }
        state->regs[rj] = state->regs[rj] - (PTTK91_NUM_REGS - 1);
        break;

    /* System calls */
    case PTTK91_SVC:
        if (param == SVC_HALT) {
#if VM_DEBUG == 1
            printf("SVC halt\n");
#endif
            state->running = 0;
        } else {
#if VM_DEBUG == 1
            printf("SVC %i\n", param);
#endif
            if (svc_handler(state, mem, param)) {
                return VM_ERR_ILLEGAL_SVC;
            }
        }
        break;
    default:
        return VM_ERR_INVALID_OPCODE;
    }

    return 0;
}

/**
 * Print all registers.
 */
static void showRegs(const struct vm_state * state)
{
    int i;
    printf("regs = ");
    for(i = 0; i < PTTK91_NUM_REGS; i++)
        printf( "r%i: %08X, ", i, (unsigned int)(state->regs[i]));
    printf("PC: %i \n", state->pc);
}

/**
 * Run program from memory.
 * @param state virtual machine state registers.
 * @param mem program memory space.
 * @param memsize size of program memory space.
 */
void vm_run(struct vm_state * state, uint32_t * mem)
{
    uint32_t instr = 0;
    int error_code;
    int rstate = 0;

    do {
        switch (rstate) {
        case 0: /* Fetch */
#if VM_DEBUG == 1
            showRegs(state);
#endif
            error_code = fetch(&instr, state, mem);
        break;

        case 1: /* Decode */
            error_code = decode(state, instr);
            break;

        case 2: /* Evaluate */
            error_code = eval(state, mem);
            break;

        default:
            error_code = 0;
            break;
        }
        /* Halt on runtime error */
        if (error_code != 0) {
            print_error_msg(error_code);
            state->running = 0;
        }

        if (++rstate > 2)
            rstate = 0;
    } while (state->running);

#if VM_DEBUG == 1
    showRegs(state);
#endif
 }

/**
  * @}
  */

