#include <stdio.h>
#include <stdint.h>

#include "elf_loader.h"

void elf_loader_read_instrs(uint32_t * buff, FILE * fp, unsigned long start, unsigned long end);

/* TODO Read symbols/ds/dc etc. */

void elf_loader_read_file(uint32_t * mem, int memsize, char * name)
{
    FILE * fp;
    unsigned long len;

    /* Open ELF binary file */
    fp = fopen(name, "rb");
    if (!fp)
    {
        fprintf(stderr, "Unable to open file %s", name);
        return;
    }

    /* Get file length */
    /* TODO Possibly not needed if elf header is handled properly */
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (memsize <= len) {
        fprintf(stderr, "Memsize too small!");
        return;
    }

    /* Read instructins */
    /** TODO Read .text offset and size */
    elf_loader_read_instrs(mem, fp, 0x5a, len - 1);

    fclose(fp);
}

void elf_loader_read_instrs(uint32_t * buff, FILE * fp, unsigned long start, unsigned long end)
{
    unsigned long i;
    unsigned char bytes[4];

    fseek(fp, start, SEEK_SET);
    for (i = 0; i <= end; i++) {
        fread(bytes, 4, 1, fp);
        buff[i] = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    }
}
