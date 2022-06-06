// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "core/core.h"
#include "core/cpu.h"
#include "core/ram.h"
#include "core/rom.h"
#include "core/ssu.h"

// =============================================================================
// Public functions definitions
// =============================================================================
int coreReset(void) {
    cpuReset();
    ramReset();
    ssuReset();

    return 0;
}
