#ifndef __INC_CORE_RAM_H__
#define __INC_CORE_RAM_H__

// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>

// =============================================================================
// Public functions declarations
// =============================================================================
/**
 * @brief Resets the RAM module.
 */
void ramReset(void);

/**
 * @brief Reads a byte from RAM.
 *
 * @param[in] p_address The address to read from.
 *
 * @returns The byte at the given address.
 */
uint8_t ramRead8(uint16_t p_address);

/**
 * @brief Reads a word from RAM.
 *
 * @param[in] p_address The address to read from.
 *
 * @returns The word at the given address.
 */
uint16_t ramRead16(uint16_t p_address);

/**
 * @brief Writes a byte to RAM.
 *
 * @param[in] p_address The address to write the byte to.
 * @param[in] p_value The byte to write.
 */
void ramWrite8(uint16_t p_address, uint8_t p_value);

/**
 * @brief Writes a word to RAM.
 *
 * @param[in] p_address The address to write the word to.
 * @param[in] p_value The word to write.
 */
void ramWrite16(uint16_t p_address, uint16_t p_value);

#endif // __INC_CORE_RAM_H__
