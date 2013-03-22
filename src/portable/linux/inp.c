#include <stdio.h>
#include "inp.h"

#define INP_KBD 0x1

int inp(int device, int * ret_val)
{
    int ch, err;
    *ret_val = 0;

    if (device == INP_KBD) {
        do {
            printf("KBD Input: ");
            //while ((ch = getchar()) != '\n' && ch != EOF);
            err = !scanf("%i", ret_val);
        } while (err);
    } else {
        return 1;
    }
    return 0;
}
