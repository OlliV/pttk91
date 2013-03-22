/* file test_vm_arit_inp_outp.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "punit.h"
#include "config.h"
#include "vm.h"

#define print_conf(conf) printf("--Note: %s = %i\n", #conf, conf)

uint32_t mem[1024];
int memsize;

/* Stdin writer */
FILE * stdin_writer;
int my_pipe[2];

static void open_stdin_writer();
static void write_stdin(char * str);

static void setup()
{
    memsize = sizeof(mem) / sizeof(uint32_t);
    memset(mem, 0x0, memsize);

    /* Open pipe */
    if (pipe(my_pipe) == -1) {
        fprintf(stderr, "Pipe failed.");
        exit(1);
    }
}

static void teardown()
{
    /* Close pipe */
    close(my_pipe[0]);
    close(my_pipe[1]);
}

static char * test_pow()
{
    int err = 0;
    int code_size;
    struct vm_state state;
    char * input[] = {"2\n", "4\n"};

    /* Write input values to stdin */
    open_stdin_writer();
    write_stdin(input[0]);
    write_stdin(input[1]);
    fclose(stdin_writer);

    b91_loader_read_file(mem, memsize, &code_size, "asm/pow.b91");
    pu_assert("Error while loading a b91 binary file.", err == 0);

    vm_init_state(&state, code_size, memsize);
    vm_run(&state, mem);

    pu_assert("Result of 2^4 == 16", state.regs[1] == 16);
    return 0;
}

static char * test_arrinit()
{
    int err = 0;
    int code_size;
    struct vm_state state;
    char input[] = "3\n";

    /* Write input values to stdin */
    open_stdin_writer();
    write_stdin(input);
    fclose(stdin_writer);

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
    pu_run_test(test_pow);
    pu_run_test(test_arrinit);
}

int main(int argc, char **argv)
{
    return pu_run_tests(&all_tests);
}

static void open_stdin_writer()
{
    dup2(my_pipe[0], STDIN_FILENO);
    stdin_writer = fdopen(my_pipe[1], "w");
}

static void write_stdin(char * str)
{
    fwrite(str, 1, strlen(str), stdin_writer);
}
