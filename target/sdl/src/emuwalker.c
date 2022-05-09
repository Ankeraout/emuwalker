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
#include "frontend/frontend.h"

// =============================================================================
// Private constants declaration
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
 * @brief This variable stores a pointer to the FLASH ROM file path.
 */
static const char *s_flashRomFilePath;

/**
 * @brief This variable stores a pointer to the EEPROM file path.
 */
static const char *s_eepromFilePath;

// =============================================================================
// Private functions declarations
// =============================================================================
/**
 * @brief Parses the command-line parameters and checks that they are valid.
 *
 * @param[in] p_argc The number of command-line parameters.
 * @param[in] p_argv The command line parameters.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
static int readCommandLineParameters(int p_argc, const char *p_argv[]);

/**
 * @brief Reads the FLASH ROM file and loads its contents into the core.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
static int loadFlashRom(void);

/**
 * @brief Reads the EEPROM file and loads its contents into the core.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
static int loadEeprom(void);

/**
 * @brief Reads the given file.
 *
 * @param[in] p_filePath The path to the file to load.
 * @param[out] p_buffer A pointer to the variable that will store the pointer to
 *                      the buffer.
 * @param[in, out] p_size Contains the maximum file size on call, and will
 *                        be replaced by the actual file size on return.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
static int readFile(const char *p_filePath, void **p_buffer, size_t *p_size);

// =============================================================================
// Public functions declarations
// =============================================================================
/**
 * @brief Entry point of the application.
 *
 * @param[in] p_argc The number of command-line parameters.
 * @param[in] p_argv The command line parameters.
 *
 * @returns An integer that indicates the result of the execution of the
 *          application.
 * @retval 0 if the execution was successful.
 * @retval Any other value if an error occurred.
 */
int main(int p_argc, const char *p_argv[]);

// =============================================================================
// Public functions definitions
// =============================================================================
int main(int p_argc, const char *p_argv[]) {
    M_UNUSED_PARAMETER(p_argc);
    M_UNUSED_PARAMETER(p_argv);

    int l_returnValue = EXIT_SUCCESS;

    if(
        (readCommandLineParameters(p_argc, p_argv) != 0)
        || (corePreinit() != 0)
        || (loadFlashRom() != 0)
        || (loadEeprom() != 0)
        || (coreInit() != 0)
        || (frontendInit() != 0)
    ) {
        l_returnValue = EXIT_FAILURE;
    }

    if(l_returnValue != EXIT_FAILURE) {
        while(true) {
            coreStep();
        }
    }

    return l_returnValue;
}

// =============================================================================
// Private functions definitions
// =============================================================================
static int readCommandLineParameters(int p_argc, const char *p_argv[]) {
    bool l_flagRom = false;
    bool l_flagEeprom = false;
    int l_returnValue = 0;

    s_flashRomFilePath = NULL;
    s_eepromFilePath = NULL;

    for(int l_argIndex = 1; l_argIndex < p_argc; l_argIndex++) {
        if(l_flagRom) {
            s_flashRomFilePath = p_argv[l_argIndex];
            l_flagRom = false;
        } else if(l_flagEeprom) {
            s_eepromFilePath = p_argv[l_argIndex];
            l_flagEeprom = false;
        } else if(strcmp(p_argv[l_argIndex], "--rom") == 0) {
            l_flagRom = true;
        } else if(strcmp(p_argv[l_argIndex], "--eeprom") == 0) {
            l_flagEeprom = true;
        }
    }

    if(l_flagRom) {
        l_returnValue = 1;
        fprintf(stderr, "Error: expected file path after \"--rom\".\n");
    } else if(l_flagEeprom) {
        l_returnValue = 1;
        fprintf(stderr, "Error: expected file path after \"--eeprom\".\n");
    } else if(s_flashRomFilePath == NULL) {
        l_returnValue = 1;
        fprintf(stderr, "Error: ROM file not specified.\n");
    } else if(s_eepromFilePath == NULL) {
        l_returnValue = 1;
        fprintf(stderr, "Error: EEPROM file not specified.\n");
    }

    return l_returnValue;
}

static int loadFlashRom(void) {
    void *l_buffer;
    size_t l_bufferSize = C_FLASH_ROM_SIZE_BYTES;

    if(readFile(s_flashRomFilePath, &l_buffer, &l_bufferSize) != 0) {
        return 1;
    }

    if(l_bufferSize != C_FLASH_ROM_SIZE_BYTES) {
        free(l_buffer);
        return 1;
    }

    int l_returnValue = coreLoadFile(E_CORE_FILE_FLASH_ROM, (uint8_t *)l_buffer, l_bufferSize);

    if(l_returnValue != 0) {
        free(l_buffer);
    }

    return l_returnValue;
}

static int loadEeprom(void) {
    void *l_buffer;
    size_t l_bufferSize = C_EEPROM_SIZE_BYTES;

    if(readFile(s_eepromFilePath, &l_buffer, &l_bufferSize) != 0) {
        return 1;
    }

    int l_returnValue = coreLoadFile(E_CORE_FILE_EEPROM, (uint8_t *)l_buffer, l_bufferSize);

    if(l_returnValue != 0) {
        free(l_buffer);
    }

    return l_returnValue;
}

static int readFile(const char *p_filePath, void **p_buffer, size_t *p_size) {
    FILE *l_file = fopen(p_filePath, "rb");

    // Get file size
    if(l_file == NULL) {
        return 1;
    }

    fseek(l_file, 0L, SEEK_END);

    size_t l_fileSize = ftell(l_file);

    // Compare file size to the maximum file size
    if(*p_size < l_fileSize) {
        fclose(l_file);
        return 1;
    }

    // Allocate a buffer for reading the file
    uint8_t *l_buffer = (uint8_t *)malloc(l_fileSize);

    if(l_buffer == NULL) {
        fclose(l_file);
        return 1;
    }

    // Read the file
    fseek(l_file, 0L, SEEK_SET);

    if(fread(l_buffer, 1, l_fileSize, l_file) != l_fileSize) {
        fclose(l_file);
        free(l_buffer);
        return 1;
    }

    fclose(l_file);

    *p_buffer = (void *)l_buffer;
    *p_size = l_fileSize;

    return 0;
}
