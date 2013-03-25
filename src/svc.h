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

/* Portable SVC calls */
#define SVC_LIB     0x1 /*!< Native library call */
#define SVC_HALT    0xb /*!< Halt program */
#define SVC_READ    0xc
#define SVC_WRITE   0xd
#define SVC_TIME    0xe
#define SVC_DATA    0xf
/* End of Portbale SVC calls */

/* Portable functions */
/**
 * Portable SVC handler.
 * @param state of virtual machine.
 * @param mem pointer to the memory of the vm.
 * @param call_code svc call code.
 * @return error code, zero if no error.
 */
int svc_handler(struct vm_state * state,  uint32_t * mem, int call_code);
/* End of portable functions */

#ifndef VM_PLATFORM
#error Please select VM_PLATFORM
#endif

#endif /* SVC_H */
