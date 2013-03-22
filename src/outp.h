#ifndef OUTP_H
#define OUTP_H

#include "pttk91.h"
#include "config.h"

/* Portable OUT devices */
#define OUTP_CRT    0
/* End of Portbale devices */

/* Portable functions */
int outp(int device, int value);
/* End of portable functions */

#ifndef VM_PLATFORM
#error Please select VM_PLATFORM
#endif

#endif /* OUTP_H */

