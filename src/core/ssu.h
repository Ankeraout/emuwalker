#ifndef __INC_CORE_SSU_H__
#define __INC_CORE_SSU_H__

// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>

// =============================================================================
// Public functions declarations
// =============================================================================
/**
 * @brief Resets the SSU module.
 */
void ssuReset(void);

/**
 * @brief Performs one cycle/clock of the SSU module.
 */
void ssuCycle(void);

/**
 * @brief Reads a byte from SSU.
 *
 * @param[in] p_address The address to read the byte from.
 *
 * @returns The byte read.
 */
uint8_t ssuRead8(uint16_t p_address);

/**
 * @brief Reads a word from SSU.
 *
 * @param[in] p_address The address to read the word from.
 *
 * @returns The word read.
 */
uint16_t ssuRead16(uint16_t p_address);

/**
 * @brief Writes a byte to SSU.
 *
 * @param[in] p_address The address to write the byte to.
 * @param[in] p_value The byte to write.
 */
void ssuWrite8(uint16_t p_address, uint8_t p_value);

/**
 * @brief Writes a word to SSU.
 *
 * @param[in] p_address The address to write the word to.
 * @param[in] p_value The word to write.
 */
void ssuWrite16(uint16_t p_address, uint16_t p_value);

#endif // __INC_CORE_SSU_H__
