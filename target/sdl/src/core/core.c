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

// =============================================================================
// Constants declaration
// =============================================================================
/**
 * @brief This constant defines the size of the FLASH ROM.
 */
#define C_FLASH_ROM_SIZE_BYTES 49152

/**
 * @brief This constant defines the size of the EEPROM.
 */
#define C_EEPROM_SIZE_BYTES 65536

// =============================================================================
// Private variables declarations
// =============================================================================
/**
 * @brief This variable stores a pointer to the FLASH ROM buffer.
 */
static uint8_t *s_flashRomBuffer;

/**
 * @brief This variable stores a pointer to the EEPROM buffer.
 */
static uint8_t *s_eepromBuffer;

// =============================================================================
// Private functions declarations
// =============================================================================

// =============================================================================
// Public functions definitions
// =============================================================================
int corePreinit(void) {
    s_flashRomBuffer = NULL;
    s_eepromBuffer = NULL;

    return 0;
}

int coreInit(void) {
    return 0;
}

int coreReset(void) {
    return 0;
}

int coreLoadFile(
    enum te_coreFile p_coreFile,
    uint8_t *p_buffer,
    size_t p_size
) {
    bool l_error = false;

    switch(p_coreFile) {
        case E_CORE_FILE_FLASH_ROM:
            if(p_size != C_FLASH_ROM_SIZE_BYTES) {
                l_error = true;
                fprintf(stderr, "Error: invalid FLASH ROM file size.\n");
            } else {
                s_flashRomBuffer = p_buffer;
            }

            break;

        case E_CORE_FILE_EEPROM:
            if(p_size != C_EEPROM_SIZE_BYTES) {
                l_error = true;
                fprintf(stderr, "Error: invalid EEPROM file size.\n");
            } else {
                s_eepromBuffer = p_buffer;
            }

            break;

        default:
            fprintf(
                stderr,
                "Error: invalid core file type (%d).\n",
                p_coreFile
            );
            l_error = true;
            break;
    }

    int l_returnValue;

    if(l_error) {
        l_returnValue = 1;
    } else {
        l_returnValue = 0;
    }

    return l_returnValue;
}

int coreSaveFile(
    enum te_coreFile p_coreFile,
    uint8_t *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_coreFile);
    M_UNUSED_PARAMETER(p_buffer);
    M_UNUSED_PARAMETER(p_size);

    // TODO

    return 0;
}

void coreFrameAdvance(void) {
    // TODO
}

union tu_coreRegister coreReadRegister(enum te_coreRegister p_register) {
    M_UNUSED_PARAMETER(p_register);

    // TODO

    union tu_coreRegister l_returnValue = {
        .word = 0
    };

    return l_returnValue;
}

uint8_t coreReadMemory(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    // TODO

    return 0;
}

void coreWriteRegister(
    enum te_coreRegister p_register,
    union tu_coreRegister p_value
) {
    M_UNUSED_PARAMETER(p_register);
    M_UNUSED_PARAMETER(p_value);

    // TODO
}

void coreWriteMemory(
    uint16_t p_address,
    uint8_t p_value
) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);

    // TODO
}
