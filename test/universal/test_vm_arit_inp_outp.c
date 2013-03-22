/* file test_vm.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "punit.h"
#include "config.h"
#include "vm.h"

uint32_t mem[1024];
int memsize;

#define test_init_vm(mem, prog, state, memsize) do {\
                                    memcpy((void*)mem, (void*)prog, sizeof(prog));\
                                    init_vm_state(&state, sizeof(prog) / sizeof(uint32_t), memsize);\
                                    } while(0)

#define print_conf(conf) printf("--Note: %s = %i\n", #conf, conf)

static void setup()
{
    memsize = sizeof(mem) / sizeof(uint32_t);
    memset(mem, 0x0, memsize);
}

static void teardown()
{
}

static char * test_load()
{
    struct vm_state state;
    uint32_t prog[] = { 0x02200001, /* load r1, =1 */
                        0x02480004, /* load r2, a */
                        0x02700005, /* load r3, @b */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);
    mem[4] = 0xa; /* a dc 10 */
    mem[5] = 0x4; /* b dc 4 */

    run(&state, mem);

    pu_assert("error, Load immediate : load r1, =1", state.regs[1] == 0x1);
    pu_assert("error, Load direct mem fetch : load r2, a", state.regs[2] == 0xa);
    pu_assert("error, Load indirect mem fetch : load r3, @b", state.regs[3] == 0xa);
    return 0;
}

static char * test_store()
{
    struct vm_state state;
    uint32_t prog[] = { 0x02200001, /* load r1, =1 */
                        0x01200007, /* store r1, a */
                        0x02200008, /* load r1, =b */
                        0x01200009, /* store r1, pb */
                        0x02200002, /* load r1, =2 */
                        0x01280009, /* store r1, @pb */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);

    run(&state, mem);

    pu_assert("error, Store the value of r1 to a", mem[7] == 0x1);
    pu_assert("error, Store the address of b to pb", mem[9] == 0x8);
    pu_assert("error, Store the value of r1 to an address pointed by pb", mem[8] == 0x2);
    return 0;
}


static char * test_code_prot()
{
    struct vm_state state;
    uint32_t prog[] = { 0x02480004, /* load r2, v */
                        0x01400002, /* store r2, tst */
                        0x02200001, /* tst load r1, =1 */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);
    mem[4] = 0x02200002; /* load r1, =2 */

    print_conf(VM_CODE_AREA_RW);

    run(&state, mem);

#if VM_CODE_AREA_RW == 0
    pu_assert("error, Code memory area protection failed.", state.regs[1] == 0x0);
#else
    pu_assert("error, Code memory area should be in writable.", state.regs[1] == 0x2);
#endif
    return 0;
}

static char * test_pc_prot()
{
    struct vm_state state;
    uint32_t prog[] = { 0x02200001 /* load r1, =1 */
                      };
    test_init_vm(mem, prog, state, memsize);
    mem[3] = 0x02200002; /* load r1, =2 */
    mem[4] = 0x70c0000b; /* svc sp, =halt */

    print_conf(VM_DATA_ALLOW_PC);

    run(&state, mem);

#if VM_DATA_ALLOW_PC == 0
    pu_assert("error, PC should not run into data area.", state.pc <= 0x2);
    pu_assert("error, PC should not run into data area.", state.regs[1] == 0x1);
#else
    pu_assert("error, It should be possible to set PC into data area.", state.pc > 0x2);
    pu_assert("error, It should be possible to run code stored in data area.", state.regs[1] == 0x2);
#endif
    return 0;
}

static char * test_push()
{
    struct vm_state state;
    uint32_t prog[] = { 0x33c00005, /* push sp, =5 */
                        0x02460000, /* load r2, sp */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);

    run(&state, mem);

    pu_assert("error, Expected push to store a value 5", mem[3] == 0x5);
    pu_assert("error, Expected push to increment the sp value", state.regs[2] == 0x3);
    return 0;
}

static char * test_pop()
{
    struct vm_state state;
    uint32_t prog[] = { 0x02c00004, /* load sp, =4 */
                        0x34c10000, /* pop sp, r1 */
                        0x02460000, /* load r2, sp */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);
    mem[4] = 0x1; /* a dc 1 */

    run(&state, mem);

    pu_assert("error, Expected pop to load a value 1", state.regs[1] == 0x1);
    pu_assert("error, Expected pop to decrement the sp value", state.regs[2] == 0x3);
    return 0;
}

static char * test_pushr()
{
    struct vm_state state;
    uint32_t prog[] = { 0x02200001, /* load r1, =1 */
                        0x02400002, /* load r2, =2 */
                        0x02600003, /* load r3, =3 */
                        0x02800004, /* load r4, =4 */
                        0x02a00005, /* load r5, =5 */
                        0x35c80000, /* pushr, sp */
                        0x02260000, /* load r1, sp */
                        0x02470000, /* load r2, fp */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);

    run(&state, mem);

    pu_assert("error, Expected pushr to store 7 values", (state.regs[1] - state.regs[2]) == 7);
    pu_assert("error, Expected pushr to push =1 as a second value in the stack", mem[10] == 1);
    pu_assert("error, Expected =5 in mem[14]", mem[14] == 5);
    return 0;
}

static char * test_popr()
{
    struct vm_state state;
    uint32_t prog[] = { 0x02200001, /* load r1, =1 */
                        0x02400002, /* load r2, =2 */
                        0x02600003, /* load r3, =3 */
                        0x02800004, /* load r4, =4 */
                        0x02a00005, /* load r5, =5 */
                        0x35c80000, /* pushr, sp */
                        0x02260000, /* load r1, sp */
                        0x02470000, /* load r2, fp */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);

    run(&state, mem);

    pu_assert("error, Expected pushr to store 7 values", (state.regs[1] - state.regs[2]) == 7);
    pu_assert("error, Expected pushr to push =1 as a second value in the stack", mem[10] == 1);
    pu_assert("error, Expected =5 in mem[14]", mem[14] == 5);
    return 0;
}

static char * test_call()
{
    struct vm_state state;
    uint32_t prog[] = { 0x33c003e8, /* push sp, =1000 */
                        0x31c00003, /* call sp, test */
                        0x70c0000b, /* svc sp, =halt */
                        0x35c80000, /* test pushr sp */
                        0x02470000, /* load r2, fp */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);

    run(&state, mem);

    pu_assert("error, Parameter pushed correctly", mem[6] == 1000);
    pu_assert("error, Return address pushed correctly by call instr", mem[7] == 0x2);
    pu_assert("error, Original Frame Pointer pushed correctly by call instr", mem[8] == 0x5);
    pu_assert("error, New Stack Pointer increments to correct position", mem[15] == 0xf);
    pu_assert("error, New Frame Pointer set correctly", state.regs[2] == 0x8);
    return 0;

}

static char * test_exit()
{
    struct vm_state state;
    uint32_t prog[] = { 0x33c00001, /* push sp, =1 */
                        0x33c00002, /* push sp, =2 ; push two parameters */
                        0x31c00006, /* call sp, test */
                        0x02260000, /* load r1, sp */
                        0x02470000, /* load r2, fp */
                        0x70c0000b, /* svc sp, =halt */
                        0x02600003, /* test load r3, =3 */
                        0x32c00002, /* exit sp, =2 */
                        0x70c0000b  /* svc sp, =halt */
                      };
    test_init_vm(mem, prog, state, memsize);

    run(&state, mem);

    pu_assert("error, SP not correctly reverted back after exit", state.regs[1] == 0x8);
    pu_assert("error, FP not correctly reverted back after exit", state.regs[2] == 0x8);
    pu_assert("error, Branch after call instruction failed?", state.regs[3] == 0x3);
    return 0;

}


static void all_tests()
{
    pu_run_test(test_load);
    pu_run_test(test_store);
    pu_run_test(test_code_prot);
    pu_run_test(test_pc_prot);
    pu_run_test(test_push);
    pu_run_test(test_pop);
    pu_run_test(test_pushr);
    pu_run_test(test_call);
    pu_run_test(test_exit);
}

int main(int argc, char **argv)
{
    return pu_run_tests(&all_tests);
}
