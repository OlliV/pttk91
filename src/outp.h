#ifndef OUTP_H
#define OUTP_H

#include "pttk91.h"
#include "config.h"

/* Portable OUT devices */
#define OUTP_CRT    0
/* End of Portbale devices */

/* Portable functions */
/**
 * Portable output handler.
 * @param device code.
 * @param value parameter value.
 * @return error code, zero if no error.
 */
int outp_handler(int device, int value);
/* End of portable functions */

#ifndef VM_PLATFORM
#error Please select VM_PLATFORM
#endif

#endif /* OUTP_H */

