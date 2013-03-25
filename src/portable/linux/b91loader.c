/**
 *******************************************************************************
 * @file    b91load.c
 * @author  Olli Vanhoja
 * @brief   B91 Titokone binary file loader for the Linux port of PTTK91.
 *******************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "b91loader.h"

enum dsections {
    header,
    code_b,
    code_e,
    data_b,
    data_e,
    load,
    symbols,
    eof
};

/**
 * Load B91 binary file to the memory.
 * @param mem pointer to the memory array used by the virtual machine.
 * @param memsize size of the memory area.
 * @param code_size returns the size of the code section.
 * @param name file name.
 * @return 0 if no error; 1 if can't open the given file; 2 if out of memory.
 */
int b91_loader_read_file(uint32_t * mem, int memsize, int * code_size, const char * name)
{
    char str[80];
    FILE * pFile;
    enum dsections state = header;
    uint32_t mem_i = 0;

#if VM_DEBUG == 1
    int sym_cnt = 0;
#endif

    *code_size = 0;

    pFile = fopen(name, "r");
    if (!pFile)
    {
        fprintf(stderr, "Unable to open file %s\n", name);
        return 1;
    }

    rewind(pFile);
    while (fscanf(pFile, "%s", str) != EOF) {
        switch (state) {
        case header:
            if (strcmp("___code___", str) == 0) {
                state = code_b;
            }
            break;
        case code_b:
            /* Code/Text start address */
            /* begin = atoi(str); */
            state = code_e;
            break;
        case code_e:
            /* Code/Text end address */
            *code_size = atoi(str);
            state = load;
            break;
        case load:
            /* Code/Data section */
            if (strcmp("___data___", str) == 0) {
                state = data_b;
                break;
            } else if (strcmp("___symboltable___", str) == 0) {
                state = symbols;
                break;
            }

            if (mem_i >= memsize) {
                /* Not enough memory to load this binary */
                fclose(pFile);
                return 2;
            }

            /* Store line to the memory location */
            mem[mem_i++] = (uint32_t)atoi(str);
            break;
        case data_b:
            state = data_e;
            break;
        case data_e:
            state = load; /* Load data */
            break;
        case symbols:
            if (strcmp("___end___", str) == 0) {
                state = eof;
                break;
            }

#if VM_DEBUG == 1
            /* Print symbol */
            if (sym_cnt == 0) {
                printf("=== Symbols ===\n");
            }
            if (sym_cnt++ % 2) {
                printf("%s\n", str);
            } else {
                printf("%s\t", str);
            }
#endif
            break;
        default:
            /* EOF */
            break;
        }
    }

#if VM_DEBUG == 1
    printf("\nB91 file loaded: %s\ncode_size = %i\n", name, *code_size);
#endif

    fclose(pFile);
    return 0;
}
