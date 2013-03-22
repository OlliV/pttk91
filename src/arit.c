/**
 ******************************************************************************
 * @file    arit.c
 * @author  Olli Vanhoja
 * @brief   Artihmetic utils
 ******************************************************************************
 */

#include "arit.h"

int arithmetic_right_shift(int x, unsigned int n) {
    if (x < 0 && n > 0)
        return (int)((unsigned int)x >> n | ~(~0U >> n));
    else
        return (int)((unsigned int)x >> n);
}
