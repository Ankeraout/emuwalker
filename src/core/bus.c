// =============================================================================
// File inclusion
// =============================================================================
#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "core/ram.h"
#include "core/rom.h"
#include "core/ssu.h"

// =============================================================================
// Private type declarations
// =============================================================================
struct ts_busPeripheral {
    uint8_t (*read8)(uint16_t p_address);
    uint16_t (*read16)(uint16_t p_address);
    void (*write8)(uint16_t p_address, uint8_t p_value);
    void (*write16)(uint16_t p_address, uint16_t p_value);
};

enum te_busPeripheral {
    E_BUS_PERIPHERAL_NONE,
    E_BUS_PERIPHERAL_ROM,
    E_BUS_PERIPHERAL_RAM,
    E_BUS_PERIPHERAL_SSU
};

// =============================================================================
// Private function declarations
// =============================================================================
/**
 * @brief Gets the peripheral from the bus address.
 *
 * @param[in] p_address The bus address.
 *
 * @returns The peripheral associated to the given bus address.
 */
static inline struct ts_busPeripheral *busGetPeripheral(uint16_t p_address);

/**
 * @brief Reads a byte from open bus (0xff).
 *
 * @param[in] p_address The address to read from.
 *
 * @returns The open bus value (0xff).
 */
static uint8_t busOpenRead8(uint16_t p_address);

/**
 * @brief Reads a word from open bus (0xffff).
 *
 * @param[in] p_address The address to read from.
 *
 * @returns The open bus value (0xffff).
 */
static uint16_t busOpenRead16(uint16_t p_address);

/**
 * @brief Writes a byte to open bus.
 *
 * @param[in] p_address The address to write to.
 * @param[in] p_value The byte to write.
 */
static void busOpenWrite8(uint16_t p_address, uint8_t p_value);

/**
 * @brief Writes a word to open bus.
 *
 * @param[in] p_address The address to write to.
 * @param[in] p_value The word to write.
 */
static void busOpenWrite16(uint16_t p_address, uint16_t p_value);

// =============================================================================
// Private variable declarations
// =============================================================================
struct ts_busPeripheral s_busPeripherals[] = {
    {
        .read8 = busOpenRead8,
        .read16 = busOpenRead16,
        .write8 = busOpenWrite8,
        .write16 = busOpenWrite16
    },
    {
        .read8 = romRead8,
        .read16 = romRead16,
        .write8 = romWrite8,
        .write16 = romWrite16
    },
    {
        .read8 = ramRead8,
        .read16 = ramRead16,
        .write8 = ramWrite8,
        .write16 = ramWrite16
    },
    {
        .read8 = ssuRead8,
        .read16 = ssuRead16,
        .write8 = ssuWrite8,
        .write16 = ssuWrite16
    }
};

/**
 * @brief This table describes the bus peripherals in the 0xf020-0xf0ff memory
 *        region.
 */
static struct ts_busPeripheral *s_io1BusPeripherals[] = {
    &s_busPeripherals[E_BUS_PERIPHERAL_ROM],
    &s_busPeripherals[E_BUS_PERIPHERAL_ROM],
    &s_busPeripherals[E_BUS_PERIPHERAL_ROM],
    &s_busPeripherals[E_BUS_PERIPHERAL_ROM],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_ROM],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_SSU],
    &s_busPeripherals[E_BUS_PERIPHERAL_SSU],
    &s_busPeripherals[E_BUS_PERIPHERAL_SSU],
    &s_busPeripherals[E_BUS_PERIPHERAL_SSU],
    &s_busPeripherals[E_BUS_PERIPHERAL_SSU],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_SSU],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_SSU],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE]
};

/**
 * @brief This table describes the bus peripherals in the 0xff80-0xffff memory
 *        region.
 */
static struct ts_busPeripheral *s_io2BusPeripherals[] = {
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE],
    &s_busPeripherals[E_BUS_PERIPHERAL_NONE]
};

// =============================================================================
// Public function definitions
// =============================================================================
void busCycle(void) {
    ssuCycle();
}

uint8_t busRead8(uint16_t p_address) {
    return busGetPeripheral(p_address)->read8(p_address);
}

uint16_t busRead16(uint16_t p_address) {
    struct ts_busPeripheral *l_busPeripheral =
        busGetPeripheral(p_address & 0xfffeU);

    if(l_busPeripheral->read16 != NULL) {
        return l_busPeripheral->read16(p_address & 0xfffeU);
    } else {
        return (l_busPeripheral->read8(p_address & 0xfffeU) << 8U)
            | l_busPeripheral->read8(p_address | 0x0001U);
    }
}

uint32_t busRead32(uint16_t p_address) {
    uint16_t l_highPart = busRead16(p_address);
    uint16_t l_lowPart = busRead16(p_address + 2);

    return (l_highPart << 16) | l_lowPart;
}

void busWrite8(uint16_t p_address, uint8_t p_value) {
    busGetPeripheral(p_address)->write8(p_address, p_value);
}

void busWrite16(uint16_t p_address, uint16_t p_value) {
    struct ts_busPeripheral *l_busPeripheral =
        busGetPeripheral(p_address & 0xfffeU);

    if(l_busPeripheral->write16 != NULL) {
        l_busPeripheral->write16(p_address & 0xfffeU, p_value);
    } else {
        l_busPeripheral->write8(p_address & 0xfffeU, p_value >> 8U);
        l_busPeripheral->write8(p_address | 0x0001, p_value);
    }
}

void busWrite32(uint16_t p_address, uint32_t p_value) {
    uint16_t l_highPart = p_value >> 16;
    uint16_t l_lowPart = p_value;

    busWrite16(p_address, l_highPart);
    busWrite16(p_address + 2, l_lowPart);
}

// =============================================================================
// Private function definitions
// =============================================================================
static inline struct ts_busPeripheral *busGetPeripheral(uint16_t p_address) {
    if((p_address & 0xc000U) != 0xc000U) { // 0x0000-0xbfff: ROM
        return &s_busPeripherals[E_BUS_PERIPHERAL_ROM];
    } else if(p_address <= 0xf01fU) { // 0xc000-0xf01f: Open bus
        return &s_busPeripherals[E_BUS_PERIPHERAL_NONE];
    } else if(p_address <= 0xf0ffU) { // 0xf020-0xf0ff: IO1
        return s_io1BusPeripherals[p_address - 0xf020U];
    } else if(p_address <= 0xf77fU) { // 0xf100 - 0xf77f: Open bus
        return &s_busPeripherals[E_BUS_PERIPHERAL_NONE];
    } else if(p_address <= 0xff7fU) { // 0xf780 - 0xff7f: RAM
        return &s_busPeripherals[E_BUS_PERIPHERAL_RAM];
    } else { // 0xff80-0xffff: IO2
        return s_io2BusPeripherals[p_address & 0x00ffU];
    }
}

static uint8_t busOpenRead8(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return 0xffU;
}

static uint16_t busOpenRead16(uint16_t p_address) {
    M_UNUSED_PARAMETER(p_address);

    return 0xffU;
}

static void busOpenWrite8(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);
}

static void busOpenWrite16(uint16_t p_address, uint16_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);
}
