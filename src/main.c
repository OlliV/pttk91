#include <stdio.h>
#include <stdint.h>

#include "vm.h"

void showRegs(const struct vm_state * state);

int main( int argc, const char * argv[] )
{
    uint32_t prog[] = { 0x1120000a, 0x04200000, 0x7000000b };
    int memsize = 3;
    struct vm_state state;

    init_vm_state(&state);
    run(&state, prog, memsize);
    return 0;
}

