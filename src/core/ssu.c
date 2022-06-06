// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>
#include <string.h>

#include "core/ssu.h"

// =============================================================================
// Private constant declarations
// =============================================================================
/**
 * @brief This constant contains the address of the SSCRH register.
 */
#define C_SSU_REGADDR_SSCRH 0xf0e0

/**
 * @brief This constant contains the address of the SSCRL register.
 */
#define C_SSU_REGADDR_SSCRL 0xf0e1

/**
 * @brief This constant contains the address of the SSMR register.
 */
#define C_SSU_REGADDR_SSMR 0xf0e2

/**
 * @brief This constant contains the address of the SSER register.
 */
#define C_SSU_REGADDR_SSER 0xf0e3

/**
 * @brief This constant contains the address of the SSSR register.
 */
#define C_SSU_REGADDR_SSSR 0xf0e4

/**
 * @brief This constant contains the address of the SSRDR register.
 */
#define C_SSU_REGADDR_SSRDR 0xf0e9

/**
 * @brief This constant contains the address of the SSTDR register.
 */
#define C_SSU_REGADDR_SSTDR 0xf0eb

// =============================================================================
// Private type declarations
// =============================================================================
union tu_ssuSscrh {
    struct {
        uint8_t css : 2;
        uint8_t scks : 1;
        uint8_t solp : 1;
        uint8_t sol : 1;
        uint8_t soos : 1;
        uint8_t bide : 1;
        uint8_t mss : 1;
    } bitField;

    uint8_t byte;
};

union tu_ssuSscrl {
    struct {
        uint8_t reserved : 3;
        uint8_t csos : 1;
        uint8_t sckos : 1;
        uint8_t sres : 1;
        uint8_t ssums : 1;
        uint8_t reserved2 : 1;
    } bitField;

    uint8_t byte;
};

union tu_ssuSsmr {
    struct {
        uint8_t cks : 3;
        uint8_t reserved : 2;
        uint8_t cphs : 1;
        uint8_t cpos : 1;
        uint8_t mls : 1;
    } bitField;

    uint8_t byte;
};

union tu_ssuSser {
    struct {
        uint8_t ceie : 1;
        uint8_t rie : 1;
        uint8_t tie : 1;
        uint8_t teie : 1;
        uint8_t reserved : 1;
        uint8_t rsstp : 1;
        uint8_t re : 1;
        uint8_t te : 1;
    } bitField;

    uint8_t byte;
};

union tu_ssuSssr {
    struct {
        uint8_t ce : 1;
        uint8_t rdrf : 1;
        uint8_t tdre : 1;
        uint8_t tend : 1;
        uint8_t reserved : 2;
        uint8_t orer : 1;
        uint8_t reserved2 : 1;
    } bitField;

    uint8_t byte;
};

// =============================================================================
// Private variable declarations
// =============================================================================
/**
 * @brief This variable represents the SSCRH register. This register contains
 *        the first part of the SSU configuration. This register is ignored by
 *        this emulator because its value is always set to 0x8c by the
 *        pokéwalker ROM.
 */
static union tu_ssuSscrh s_ssuSscrh;

/**
 * @brief This variable represents the SSCRL register. This register contains
 *        the second part of the SSU configuration. This register is ignored by
 *        this emulator because its value is always set to 0x40 by the
 *        pokéwalker ROM.
 */
static union tu_ssuSscrl s_ssuSscrl;

/**
 * @brief This variable represents the SSMR register. This register contains the
 *        settings for serial data transmission.
 */
static union tu_ssuSsmr s_ssuSsmr;

/**
 * @brief This variable represents the SSER register. This register contains the
 *        transmit, receive and interrupt enable bits of the SSU.
 */
static union tu_ssuSser s_ssuSser;

/**
 * @brief This variable represents the SSSR register. This register contains the
 *        interrupt flags related to the SSU operation.
 */
static union tu_ssuSssr s_ssuSssr;

/**
 * @brief This variable represents the SSRDR register. This register contains
 *        the data copied from SSTRSR after one transaction.
 */
static uint8_t s_ssuSsrdr;

/**
 * @brief This variable represents the SSTDR register. This register contains
 *        the data to be copied into SSTRSR and sent.
 */
static uint8_t s_ssuSstdr;

// =============================================================================
// Private function declarations
// =============================================================================
/**
 * @brief Returns the value of the SSRDR register and performs side-effects.
 * @details Reading from SSRDR clears the SSSR.RDRF bit.
 *
 * @returns The value of the SSRDR register.
 */
static uint8_t ssuReadSsrdr(void);

/**
 * @brief Writes a value to the SSTDR register and performs side-effects.
 * @details Writing to SSTDR clears the SSSR.TDRE bit.
 *
 * @param[in] p_value The value to write to SSTDR.
 */
static void ssuWriteSstdr(uint8_t p_value);

// =============================================================================
// Public functions definitions
// =============================================================================
void ssuReset(void) {
    s_ssuSscrh.byte = 0x08;
    s_ssuSscrl.byte = 0x00;
    s_ssuSsmr.byte = 0x00;
    s_ssuSser.byte = 0x00;
    s_ssuSssr.byte = 0x04;
    s_ssuSsrdr = 0x00;
    s_ssuSstdr = 0x00;
}

uint8_t ssuRead8(uint16_t p_address) {
    switch(p_address) {
        case C_SSU_REGADDR_SSCRH: return s_ssuSscrh.byte;
        case C_SSU_REGADDR_SSCRL: return s_ssuSscrl.byte & 0x78;
        case C_SSU_REGADDR_SSMR: return s_ssuSsmr.byte & 0xe7;
        case C_SSU_REGADDR_SSER: return s_ssuSser.byte & 0xef;
        case C_SSU_REGADDR_SSSR: return s_ssuSssr.byte & 0x4f;
        case C_SSU_REGADDR_SSRDR: return ssuReadSsrdr();
        case C_SSU_REGADDR_SSTDR: return s_ssuSstdr;
        default: return 0xff;
    }
}

uint16_t ssuRead16(uint16_t p_address) {
    return 0xff00 | ssuRead8(p_address);
}

void ssuWrite8(uint16_t p_address, uint8_t p_value) {
    switch(p_address) {
        case C_SSU_REGADDR_SSCRH: s_ssuSscrh.byte = p_value; break;
        case C_SSU_REGADDR_SSCRL: s_ssuSscrl.byte = p_value; break;
        case C_SSU_REGADDR_SSMR: s_ssuSsmr.byte = p_value; break;
        case C_SSU_REGADDR_SSER: s_ssuSser.byte = p_value; break;
        case C_SSU_REGADDR_SSSR: s_ssuSssr.byte &= p_value; break;
        case C_SSU_REGADDR_SSRDR: break;
        case C_SSU_REGADDR_SSTDR: ssuWriteSstdr(p_value); break;
        default: break;
    }
}

void ssuWrite16(uint16_t p_address, uint16_t p_value) {
    ssuWrite8(p_address, p_value);
}

void ssuCycle(void) {

}

// =============================================================================
// Private functions definitions
// =============================================================================
static uint8_t ssuReadSsrdr(void) {
    s_ssuSssr.bitField.rdrf = 0;
    return s_ssuSsrdr;
}

static void ssuWriteSstdr(uint8_t p_value) {
    s_ssuSssr.bitField.tdre = 0;
    s_ssuSstdr = p_value;
}
