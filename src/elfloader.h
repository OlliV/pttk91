/**
 *******************************************************************************
 * @file    elfloader.h
 * @author  Olli Vanhoja
 * @brief   Elf binary loader.
 *******************************************************************************
 */

#ifndef ELFLOADER_H
#define ELFLOADER_H
void elf_loader_read_file(uint32_t * mem, int memsize, char * name);
#endif
