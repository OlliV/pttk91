#ifndef SVC_H
#define SVC_H

#include "pttk91.h"
#include "config.h"

/* Portable SVC calls */
#define SVC_LIB     0x1 /** Native library call */
#define SVC_HALT    0xb /** Halt program */
#define SVC_READ    0xc
#define SVC_WRITE   0xd
#define SVC_TIME    0xe
#define SVC_DATA    0xf
/* End of Portbale SVC calls */

#ifndef VM_PLATFORM
#error Please select VM_PLATFORM
#endif

#endif /* SVC_H */
