#ifndef OUTP_H
#define OUTP_H

#include "pttk91.h"
#include "config.h"

/* Portable OUT devices */
#define OUTP_CRT    0
/* End of Portbale devices */

/* Portable functions */
void outp_crt(int value);
/* End of portable functions */

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

#endif /* OUTP_H */

