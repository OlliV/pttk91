/**
 *******************************************************************************
 * @file    svc.h
 * @author  Olli Vanhoja
 * @brief   SVC header file.
 *******************************************************************************
 */

#ifndef SVC_H
#define SVC_H

#include <stdint.h>
#include "pttk91.h"
#include "vm.h"
#include "config.h"

#define svc_lib     0x0a    /*!< Native library call */
#define svc_halt    0x0b    /*!< Halt program */
#define svc_read    0x0c
#define svc_write   0x0d
#define svc_time    0x0e
#define svc_date    0x0f

/** For all SVCs; X Macro */
#define FOR_ALL_SVC(apply) \
    apply(svc_lib)         \
    apply(svc_halt)        \
    apply(svc_read)        \
    apply(svc_write)       \
    apply(svc_time)        \
    apply(svc_date)

#ifndef VM_PLATFORM
#error Please select VM_PLATFORM
#endif

typedef void (*svc_handler_t)(struct vm_state * state,  uint32_t * mem);

/**
 * Generic SVC handler.
 * @param state of virtual machine.
 * @param mem pointer to the memory of the vm.
 * @param call_code svc call code.
 * @return error code, zero if no error.
 */
int svc_handler(struct vm_state * state,  uint32_t * mem, int call_code);

/* Portable functions */
void svc_lib_fn(struct vm_state * state, uint32_t * mem);
void svc_read_fn(struct vm_state * state, uint32_t * mem);
void svc_write_fn(struct vm_state * state, uint32_t * mem);
void svc_time_fn(struct vm_state * state, uint32_t * mem);
void svc_date_fn(struct vm_state * state, uint32_t * mem);
/* End of portable functions */

#endif /* SVC_H */
