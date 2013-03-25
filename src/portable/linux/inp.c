/**
 *******************************************************************************
 * @file    inp.c
 * @author  Olli Vanhoja
 * @brief   Input handler for the Linux port of PTTK91.
 *******************************************************************************
 */

#include <stdio.h>
#include "inp.h"

#define INP_KBD 0x1

int inp_handler(int device, int * ret_val)
{
    int ch, err;
    *ret_val = 0;

    if (device == INP_KBD) {
        do {
            printf("KBD Input: ");
            err = !scanf("%i", ret_val);
            while( (ch = fgetc( stdin )) != EOF && ch != '\n' );
        } while (err);
    } else {
        return 1;
    }
    return 0;
}
