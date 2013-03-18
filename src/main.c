#include <stdio.h>
#include <stdint.h>

#include "vm.h"
#include "elf_loader.h"

void showRegs(const struct vm_state * state);

int main( int argc, const char * argv[] )
{
    //uint32_t prog[] = { 0x1120000a, 0x04200000, 0x7000000b };
    uint32_t mem[512];
    int memsize = 512;
    struct vm_state state;

    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    elf_loader_read_file(mem, memsize, argv[1]);

    init_vm_state(&state);
    run(&state, mem, memsize);

    return 0;
}

