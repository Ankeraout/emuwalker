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

/**
 * @brief This variable represents the SSTRSR register. This register is a shift
 *        register that contains the data to be transmitted as well as the
 *        received data. Note that this register is an internal register that
 *        cannot be accessed by the CPU.
 */
static uint8_t s_ssuSstrsr;

/**
 * @brief This variable contains the clock counter for the prescaler.
 * @details One SSU clock occurs when this variable reaches a value >= 256.
 */
static int s_ssuClockCounter;

/**
 * @brief This variable contains the bit counter for the current transfer.
 * @details The transfer ends when this variable reaches 8.
 */
static int s_ssuBitCounter;

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
    s_ssuClockCounter = 0;
    s_ssuBitCounter = 0;
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
    // If a transfer is in progress
    if(s_ssuSssr.bitField.tend == 0) {
        s_ssuClockCounter += 1 << s_ssuSsmr.bitField.cks;

        if(s_ssuClockCounter >= 256) {
            s_ssuClockCounter -= 256;

            s_ssuBitCounter++;

            if(s_ssuBitCounter == 8) {
                // TODO: Actually perform the transfer

                if(s_ssuSssr.bitField.tdre == 0) {
                    // If there is data in the buffer, keep transferring data.
                    s_ssuSstrsr = s_ssuSstdr;
                    s_ssuSssr.bitField.tdre = 1;
                } else {
                    // Otherwise stop the transfer.
                    s_ssuSssr.bitField.tend = 1;
                }

                if(s_ssuSssr.bitField.rdrf == 1) {
                    // If there is still data left in SSRDR, then the new data
                    // is lost, and an error flag is set.
                    s_ssuSssr.bitField.orer = 1;
                } else {
                    // Otherwise SSRDR contains the received data.
                    // TODO: Return the actual received data from the transfer.
                    s_ssuSsrdr = 0xff;
                }

                s_ssuBitCounter = 0;
            }
        }
    }
}

// =============================================================================
// Private functions definitions
// =============================================================================
static uint8_t ssuReadSsrdr(void) {
    s_ssuSssr.bitField.rdrf = 0;
    return s_ssuSsrdr;
}

static void ssuWriteSstdr(uint8_t p_value) {
    s_ssuSstdr = p_value;

    if(s_ssuSssr.bitField.tend == 1) {
        // If no transfer is in progress, initiate a new transfer.
        s_ssuSstrsr = s_ssuSstdr;
        s_ssuSssr.bitField.tend = 0;
    } else {
        // If a transfer is already in progress, put the value in the buffer.
        s_ssuSssr.bitField.tdre = 0;
    }
}
