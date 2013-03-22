/* file test_vm_arit_inp_outp.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "punit.h"
#include "config.h"
#include "vm.h"

uint32_t mem[1024];
int memsize;


#define print_conf(conf) printf("--Note: %s = %i\n", #conf, conf)

static void setup()
{
    memsize = sizeof(mem) / sizeof(uint32_t);
    memset(mem, 0x0, memsize);
}

static void teardown()
{
}

static char * test_pow()
{
    int err = 0;
    int code_size;
    struct vm_state state;
    int my_pipe[2];
    char * input[] = {"2\n", "4\n"};
    FILE * stdin_writer;

    err = (pipe(my_pipe) == -1);
    pu_assert("Pipe failed.", err != -1);
    dup2(my_pipe[0], STDIN_FILENO);
    stdin_writer = fdopen(my_pipe[1], "w");

    /* Write input values to stdin */
    fwrite(input[0], 1, strlen(input[0]), stdin_writer);
    fwrite(input[1], 1, strlen(input[1]), stdin_writer);
    fclose(stdin_writer);
    close(my_pipe[0]);
    close(my_pipe[1]);

    b91_loader_read_file(mem, memsize, &code_size, "asm/pow.b91");
    pu_assert("Error while loading a b91 binary file.", err == 0);

    vm_init_state(&state, code_size, memsize);
    vm_run(&state, mem);

    pu_assert("Result of 2^4 == 16", state.regs[1] == 16);
    return 0;
}

static void all_tests()
{
    pu_run_test(test_pow);
}

int main(int argc, char **argv)
{
    return pu_run_tests(&all_tests);
}
