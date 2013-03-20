/* file test_vm.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "punit.h"
#include "config.h"
#include "vm.h"

uint32_t mem[1024];
int memsize;

#define test_init_vm(mem, prog, state) do {\
                                    memcpy((void*)mem, (void*)prog, sizeof(prog));\
                                    init_vm_state(&state, sizeof(prog) / sizeof(uint32_t));\
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
    test_init_vm(mem, prog, state);
    mem[4] = 0xa; /* a dc 10 */
    mem[5] = 0x4; /* b dc 4 */

    run(&state, mem, memsize);

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
    test_init_vm(mem, prog, state);

    run(&state, mem, memsize);

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
    test_init_vm(mem, prog, state);
    mem[4] = 0x02200002; /* load r1, =2 */

    print_conf(VM_CODE_AREA_RW);

    run(&state, mem, memsize);

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
    test_init_vm(mem, prog, state);
    mem[3] = 0x02200002; /* load r1, =2 */
    mem[4] = 0x70c0000b; /* svc sp, =halt */

    print_conf(VM_DATA_ALLOW_PC);

    run(&state, mem, memsize);

#if VM_DATA_ALLOW_PC == 0
    pu_assert("error, PC should not run into data area.", state.pc <= 0x2);
    pu_assert("error, PC should not run into data area.", state.regs[1] == 0x1);
#else
    pu_assert("error, It should be possible to set PC into data area.", state.pc > 0x2);
    pu_assert("error, It should be possible to run code stored in data area.", state.regs[1] == 0x2);
#endif
    return 0;
}

static void all_tests()
{
    pu_run_test(test_load);
    pu_run_test(test_store);
    pu_run_test(test_code_prot);
    pu_run_test(test_pc_prot);
}

int main(int argc, char **argv)
{
    return pu_run_tests(&all_tests);
}
