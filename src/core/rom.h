#ifndef __INC_CORE_ROM_H__
#define __INC_CORE_ROM_H__

// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>

// =============================================================================
// Public function declarations
// =============================================================================
/**
 * @brief Initializes the ROM buffer.
 * 
 * @param[in,out] p_romBuffer The ROM buffer to use. Note that this buffer must
 *                            not be freed after calling this function because
 *                            this function stores a pointer to it, and does not
 *                            make a copy.
 */
void romInit(uint8_t *p_romBuffer);

/**
 * @brief Performs a reset of the FLASH ROM.
 */
void romReset(void);

/**
 * @brief Reads a byte from the FLASH ROM.
 * 
 * @param[in] p_address The address to read the byte from.
 * 
 * @returns The byte read.
 */
uint8_t romRead8(uint16_t p_address);

/**
 * @brief Reads a word from the FLASH ROM.
 * 
 * @param[in] p_address The address to read the word from.
 * 
 * @returns The word read.
 */
uint16_t romRead16(uint16_t p_address);

/**
 * @brief Writes a byte to the FLASH ROM.
 * 
 * @param[in] p_address The address to write the byte to.
 * @param[in] p_value The value to write.
 */
void romWrite8(uint16_t p_address, uint8_t p_value);

/**
 * @brief Writes a word to the FLASH ROM.
 * 
 * @param[in] p_address The address to write the word to.
 * @param[in] p_value The value to write.
 */
void romWrite16(uint16_t p_address, uint16_t p_value);

#endif
