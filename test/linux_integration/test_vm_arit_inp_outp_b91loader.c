/* file test_vm_arit_inp_outp.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "punit.h"
#include "unixunit.h"
#include "config.h"
#include "vm.h"

#define print_conf(conf) printf("--Note: %s = %i\n", #conf, conf)

uint32_t mem[1024];
int memsize;

static void setup()
{
    memsize = sizeof(mem) / sizeof(uint32_t);
    memset(mem, 0x0, memsize);

    uu_open_pipe();
}

static void teardown()
{
    uu_close_pipe();
}

static char * test_pow()
{
    int err = 0;
    int code_size;
    struct vm_state state;
    char * input[] = {"2\n", "4\n"};

    /* Write input values to stdin */
    uu_open_stdin_writer();
    uu_write_stdin(input[0]);
    uu_write_stdin(input[1]);
    uu_close_stdin_writer();

    b91_loader_read_file(mem, memsize, &code_size, "asm/pow.b91");
    pu_assert("Error while loading a b91 binary file.", err == 0);

    vm_init_state(&state, code_size, memsize);
    vm_run(&state, mem);

    pu_assert_equal("Result of 2^4 == 16", state.regs[1], 16);
    return 0;
}

static char * test_arrinit()
{
    int err = 0;
    int code_size;
    struct vm_state state;
    char input[] = "3\n";

    /* Write input values to stdin */
    uu_open_stdin_writer();
    uu_write_stdin(input);
    uu_close_stdin_writer();

    b91_loader_read_file(mem, memsize, &code_size, "asm/arrinit.b91");
    pu_assert("Error while loading a b91 binary file.", err == 0);

    vm_init_state(&state, code_size, memsize);
    vm_run(&state, mem);

    pu_assert("Correct r1 value", state.regs[1] == 3);
    pu_assert("Correct r2 value", state.regs[2] == 2716);
    pu_assert("Correct r3 value", state.regs[3] == 2716);
    pu_assert("Correct mem value at", mem[20] == -875);
    pu_assert("Correct mem value at", mem[25] == 860);
    pu_assert("Correct mem value at", mem[27] == 1554);
    return 0;
}

static void all_tests()
{
    pu_def_test(test_pow, PU_RUN);
    pu_def_test(test_arrinit, PU_RUN);
}

int main(int argc, char **argv)
{
    return pu_run_tests(&all_tests);
}
