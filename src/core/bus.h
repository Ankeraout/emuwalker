#ifndef __INC_CORE_BUS_H__
#define __INC_CORE_BUS_H__

// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>

// =============================================================================
// Public function declarations
// =============================================================================
/**
 * @brief Reads a byte from the bus.
 *
 * @param[in] p_address The address to read the byte from.
 *
 * @returns The byte read.
 */
uint8_t busRead8(uint16_t p_address);

/**
 * @brief Reads a word from the bus.
 *
 * @param[in] p_address The address to read the word from.
 *
 * @returns The word read.
 */
uint16_t busRead16(uint16_t p_address);

/**
 * @brief Reads a longword from the bus.
 *
 * @param[in] p_address The address to read the longword from.
 *
 * @returns The longword read.
 */
uint32_t busRead32(uint16_t p_address);

/**
 * @brief Writes a byte on the bus.
 *
 * @param[in] p_address The address to write the byte to.
 * @param[in] p_value The value to write.
 */
void busWrite8(uint16_t p_address, uint8_t p_value);

/**
 * @brief Writes a word on the bus.
 *
 * @param[in] p_address The address to write the word to.
 * @param[in] p_value The value to write.
 */
void busWrite16(uint16_t p_address, uint16_t p_value);

/**
 * @brief Writes a longword on the bus.
 *
 * @param[in] p_address The address to write the longword to.
 * @param[in] p_value The value to write.
 */
void busWrite32(uint16_t p_address, uint32_t p_value);

#endif
