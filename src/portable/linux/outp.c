/**
 *******************************************************************************
 * @file    outp.c
 * @author  Olli Vanhoja
 * @brief   Output handler for the Linux port of PTTK91.
 *******************************************************************************
 */

#include <stdio.h>
#include "outp.h"

int outp_handler(int device, int value) {
    if (device == OUTP_CRT) {
        printf("CRT output: %i\n", value);
    } else {
        return 1;
    }
    return 0;
}
