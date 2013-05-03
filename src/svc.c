/**
 *******************************************************************************
 * @file    svc.c
 * @author  Olli Vanhoja
 * @brief   Generic SVC handler.
 *******************************************************************************
 */

#include <stdio.h>
#include <stddef.h>
#include "svc.h"

void svc_halt_fn(struct vm_state * state, uint32_t * mem);

svc_handler_t svc_callmap[] = {
                            #define SVC_MAP_X(value) value##_fn,
                            FOR_ALL_SVC(SVC_MAP_X)
                            #undef SVC_MAP_X
                       };

int svc_handler(struct vm_state * state, uint32_t * mem, int call_code)
{
    svc_handler_t fpt;
    call_code -= 10;

    if ((call_code >= sizeof(svc_callmap) / sizeof(void *))
        || (call_code < 0)) {
        return 1;
    }

    fpt = svc_callmap[call_code];
    fpt(state, mem);

    return 0;
}

void svc_halt_fn(struct vm_state * state, uint32_t * mem)
{
#if VM_DEBUG == 1
    printf("SVC halt\n");
#endif
    state->running = 0;
}
