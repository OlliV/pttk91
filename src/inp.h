#ifndef INP_H
#define INP_H

#include "pttk91.h"
#include "config.h"

/* Portable functions */
/**
 * Portable input handler.
 * @param device code.
 * @param ret_val return value.
 * @return error code, zero if no error.
 */
int inp_handler(int device, int * ret_val);
/* End of portable functions */

#ifndef VM_PLATFORM
#error Please select VM_PLATFORM
#endif

#endif /* INP_H */

