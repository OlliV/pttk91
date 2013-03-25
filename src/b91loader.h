/**
 *******************************************************************************
 * @file    b91loader.h
 * @author  Olli Vanhoja
 * @brief   B91 Titokone binary file loader header file.
 *******************************************************************************
 */

#ifndef B91_LOADER_H
#define B91_LOADER_H
int b91_loader_read_file(uint32_t * mem, int memsize, int * code_size, const char * name);
#endif /* B91_LOADER_H */
