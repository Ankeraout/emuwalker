// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>
#include <string.h>

#include "core/ram.h"

// =============================================================================
// Private constant declarations
// =============================================================================
#define C_RAM_SIZE 2048

// =============================================================================
// Private variable declarations
// =============================================================================
static uint8_t s_ramData[C_RAM_SIZE];

// =============================================================================
// Public functions definitions
// =============================================================================
void ramReset(void) {
    memset(s_ramData, 0, C_RAM_SIZE);
}

uint8_t ramRead8(uint16_t p_address) {
    return s_ramData[p_address - 0xf780];
}

uint16_t ramRead16(uint16_t p_address) {
    return (s_ramData[p_address - 0xf780] << 8) | s_ramData[p_address - 0xf77f];
}

void ramWrite8(uint16_t p_address, uint8_t p_value) {
    s_ramData[p_address - 0xf780] = p_value;
}

void ramWrite16(uint16_t p_address, uint16_t p_value) {
    s_ramData[p_address - 0xf780] = p_value >> 8;
    s_ramData[p_address - 0xf77f] = p_value;
}
