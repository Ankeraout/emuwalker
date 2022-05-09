#ifndef __INC_CORE_CORE_H__
#define __INC_CORE_CORE_H__

// =============================================================================
// File inclusion
// =============================================================================
#include <stddef.h>
#include <stdint.h>

// =============================================================================
// Public types declarations
// =============================================================================
enum te_coreFile {
    E_CORE_FILE_FLASH_ROM,
    E_CORE_FILE_EEPROM
};

enum te_coreInput {
    E_CORE_INPUT_LEFT,
    E_CORE_INPUT_MIDDLE,
    E_CORE_INPUT_RIGHT
};

enum te_coreInputState {
    E_CORE_INPUT_RELEASED,
    E_CORE_INPUT_PRESSED
};

enum te_coreRegister {
    E_CORE_REGISTER_ER0
};

union tu_coreRegister {
    uint8_t byte;
    uint16_t word;
    uint32_t dword;
};

// =============================================================================
// Public functions declarations
// =============================================================================
/**
 * @brief Pre-initializes the core module (necessary before calling any core*
 *        function).
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
int corePreinit(void);

/**
 * @brief Initializes the core module.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
int coreInit(void);

/**
 * @brief Resets the core.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
int coreReset(void);

/**
 * @brief Loads a file buffer in the core.
 *
 * @param[in] p_coreFile The type of the file to load.
 * @param[in] p_buffer A pointer to the buffer to load. Note that the core
 *                     will make a copy of this buffer and therefore the passed
 *                     buffer's contents will not be modified.
 * @param[in] p_size The size of the buffer.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the buffer was loaded successfully.
 * @retval Any other value if an error occurred.
 */
int coreLoadFile(
    enum te_coreFile p_coreFile,
    uint8_t *p_buffer,
    size_t p_size
);

/**
 * @brief Saves a file buffer from the core.
 *
 * @param[in] p_coreFile The type of the file to save.
 * @param[out] p_buffer A pointer to the buffer to fill.
 * @param[in] p_size The size of the buffer.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the buffer was filled successfully.
 * @retval -1 if the buffer is too small.
 * @retval -2 if the file does not exist.
 */
int coreSaveFile(
    enum te_coreFile p_coreFile,
    uint8_t *p_buffer,
    size_t p_size
);

/**
 * @brief Runs the core until the next VBlank event is triggered.
 */
void coreFrameAdvance(void);

/**
 * @brief Runs the core until one CPU instruction is completely executed.
 */
void coreStep(void);

/**
 * @brief Sets the state of the given input key of the core.
 *
 * @param[in] p_input The input key to set the state of.
 * @param[in] p_inputState The new state of the given input key.
 */
void coreSetInput(
    enum te_coreInput p_input,
    enum te_coreInputState p_inputState
);

/**
 * @brief Gets a pointer to the video buffer of the core.
 *
 * @returns A pointer to the video buffer of the core.
 */
const uint32_t *coreGetVideoBuffer(void);

/**
 * @brief Returns the value of the given core register.
 *
 * @param[in] p_register The core register to read.
 *
 * @returns The value of the given core register.
 */
union tu_coreRegister coreReadRegister(enum te_coreRegister p_register);

/**
 * @brief Returns the value in memory at the given address.
 *
 * @param[in] p_address The memory address to read.
 *
 * @returns The value at the given memory address.
 */
uint8_t coreReadMemory(uint16_t p_address);

/**
 * @brief Writes the given value in the given register.
 *
 * @param[in] p_register The register to set with the given value.
 * @param[in] p_value The value to set the given register to.
 */
void coreWriteRegister(
    enum te_coreRegister p_register,
    union tu_coreRegister p_value
);

/**
 * @brief Writes the given value at the given address in the core memory address
 *        space.
 *
 * @param[in] p_address The address to write the given value to.
 * @param[in] p_value The value to write at the given address.
 */
void coreWriteMemory(
    uint16_t p_address,
    uint8_t p_value
);

#endif // __INC_CORE_CORE_H__
