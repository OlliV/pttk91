#include <stdio.h>
#include <stdint.h>

#include "vm.h"
#include "b91_loader.h"

void showRegs(const struct vm_state * state);

int main( int argc, const char * argv[] )
{
    //uint32_t prog[] = { 0x1120000a, 0x04200000, 0x7000000b };
    uint32_t mem[1024];
    int memsize = sizeof(mem)/sizeof(uint32_t);
    struct vm_state state;

    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    if(b91_loader_read_file(mem, memsize, argv[1])) {
        printf("Error while loading b91 binary.\n");
        return 2;
    }

    init_vm_state(&state);
    run(&state, mem, memsize);

    return 0;
}

