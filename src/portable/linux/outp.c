#include <stdio.h>
#include "outp.h"

int outp(int device, int value) {
    if (device == OUTP_CRT) {
        printf("CRT output: %i\n", value);
    } else {
        return 1;
    }
    return 0;
}
