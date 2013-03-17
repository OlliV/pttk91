#ifndef SVC_H
#define SVC_H

#include "pttk91.h"
#include "config.h"

/* Portable SVC calls */
#define SVC_HALT    11
/* End of Portbale SVC calls */

#ifndef VM_PLATFORM
#error Please select VM_PLATFORM
#endif

#if VM_PLATFORM == LINUX
/* Nothing to include yet */
#elif VM_PLATFORM == ZEKE
#error Zeke not supported yet.
#else
#error Platform not supported.
#endif

#endif /* SVC_H */
