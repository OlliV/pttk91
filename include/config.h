#ifndef CONFIG_H
#define CONFIG_H

#include "platforms.h"

#define VM_PLATFORM LINUX
/** Print debug messages */
#define VM_DEBUG 1
/** Allow writes in code section */
#define VM_CODE_AREA_RW 0
/** Allow program execution in data section. */
#define VM_DATA_ALLOW_PC 0

#endif /* CONFIG_H */
