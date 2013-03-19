#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#include "b91_loader.h"

enum dsections {
    header,
    code_b,
    code_e,
    code,
    data_b,
    data_e,
    data,
    symbols,
    eof
};

int b91_loader_read_file(uint32_t * mem, int memsize, const char * name)
{
    char str[80];
    FILE * pFile;
    enum dsections state;

    int begin;
    int end;
    uint32_t mem_i = 0;

    pFile = fopen(name, "r");
    if (!pFile)
    {
        fprintf(stderr, "Unable to open file %s", name);
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
            begin = atoi(str);
            state = code_e;
            break;
        case code_e:
            end = atoi(str);
            state = code;
            break;
        case code:
            if (strcmp("___data___", str) == 0) {
                state = data_b;
                break;
            }

            if (mem_i >= memsize) {
                return -1;
            }
            mem[mem_i++] = (uint32_t)atoi(str);
            break;
        case data_b:
            state = data_e;
            break;
        case data_e:
            state = data;
            break;
        case data:
            if (strcmp("___symboltable___", str) == 0) {
                state = symbols;
                break;
            }

            if (mem_i >= memsize) {
                return -1;
            }
            mem[mem_i++] = (uint32_t)atoi(str);
            break;
        case symbols:
            if (strcmp("___end___", str) == 0) {
                state = eof;
                break;
            }

#if VM_DEBUG == 1
            printf("SYM: %s\n", str);
#endif
            break;
        default:
            break;
        }
    }

#if VM_DEBUG == 1
    printf("\nbegin = %i, end = %i\n", begin, end);
#endif

    fclose(pFile);
    return 0;
}
