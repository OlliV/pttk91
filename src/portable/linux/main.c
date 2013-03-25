/**
 *******************************************************************************
 * @file    main.c
 * @author  Olli Vanhoja
 * @brief   Main soure file for the Linux port of PTTK91.
 *******************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "vm.h"
#include "b91loader.h"

int main(int argc, const char * argv[])
{
    uint32_t * mem;
    int memsize = 1024;
    int code_size;
    struct vm_state state;

    char * file_name = NULL;
    int c;

    opterr = 0;
    while ((c = getopt(argc, (char * const*)argv, "f:m:")) != -1) {
        switch (c) {
        case 'f': /* File name */
            file_name = optarg;
            break;
        case 'm': /* Amount of memory to be allocated */
            memsize = atoi(optarg);
            break;
        case '?':
            if (optopt == 'c')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(1);
        default:
            abort();
        }
    }

    mem = (uint32_t *)malloc(memsize);
    if (mem == NULL) {
        fprintf(stderr, "Can't allocate memory for the VM.\n");
        exit(2);
    }

    if (b91_loader_read_file(mem, memsize, &code_size, file_name)) {
        fprintf(stderr, "Error while loading a b91 binary file.\n");
        free(mem);
        exit(3);
    }

    vm_init_state(&state, code_size, memsize);
    printf("=== Run ===\n");
    vm_run(&state, mem);

    free(mem);
    return 0;
}
