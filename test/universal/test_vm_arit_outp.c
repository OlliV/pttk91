/* file test_vm.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "punit.h"
#include "vm.h"

uint32_t mem[1024];
int memsize;

#define test_init_vm(mem, prog, state) do {\
                                    memcpy((void*)mem, (void*)prog, sizeof(prog));\
                                    init_vm_state(&state, sizeof(mem) / sizeof(uint32_t));\
                                    } while(0)

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

static void all_tests()
{
    pu_run_test(test_load);
}

int main(int argc, char **argv)
{
    return pu_run_tests(&all_tests);
}
