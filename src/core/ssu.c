// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>
#include <string.h>

#include "core/ssu.h"

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

/*
union tu_ssu {
    struct {
    } bitField;

    uint8_t byte;
};
*/

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


// =============================================================================
// Public functions definitions
// =============================================================================
void ssuReset(void) {

}

uint8_t ssuRead8(uint16_t p_address) {

}

uint16_t ssuRead16(uint16_t p_address) {

}

void ssuWrite8(uint16_t p_address, uint8_t p_value) {

}

void ssuWrite16(uint16_t p_address, uint16_t p_value) {

}
