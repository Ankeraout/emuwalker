// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>

#include "common.h"

// =============================================================================
// Private constant declarations
// =============================================================================
#define C_ROM_SIZE_BYTES 49152U

// =============================================================================
// Private variable declarations
// =============================================================================
static uint8_t *s_romData;

// =============================================================================
// Private type declarations
// =============================================================================
union tu_flashMemoryControlRegister1 {
    uint8_t byte;

    struct {
        uint8_t program : 1;
        uint8_t erase : 1;
        uint8_t programVerify : 1;
        uint8_t eraseVerify : 1;
        uint8_t programSetup : 1;
        uint8_t eraseSetup : 1;
        uint8_t softwareWriteEnable : 1;
        uint8_t reserved : 1;
    } bitField;
};

union tu_flashMemoryControlRegister2 {
    uint8_t byte;

    struct {
        uint8_t reserved : 7;
        uint8_t flashMemoryError : 1;
    } bitField;
};

union tu_eraseBlockRegister1 {
    uint8_t byte;

    struct {
        uint8_t eraseBlock0 : 1;
        uint8_t eraseBlock1 : 1;
        uint8_t eraseBlock2 : 1;
        uint8_t eraseBlock3 : 1;
        uint8_t eraseBlock4 : 1;
        uint8_t eraseBlock5 : 1;
        uint8_t reserved : 2;
    } bitField;
};

union tu_flashMemoryPowerControlRegister {
    uint8_t byte;

    struct {
        uint8_t reserved : 7;
        uint8_t powerDownDisable : 1;
    } bitField;
};

union tu_flashMemoryEnableRegister {
    uint8_t byte;

    struct {
        uint8_t reserved : 7;
        uint8_t flashMemoryControlRegisterEnable;
    } bitField;
};

// =============================================================================
// Private function declarations
// =============================================================================
/**
 * @brief Reads the value of the FLMCR1 register.
 * 
 * @returns The value of the register.
 */
static inline uint8_t read8FlashMemoryControlRegister1(void);

/**
 * @brief Reads the value of the FLMCR2 register.
 * 
 * @returns The value of the register.
 */
static inline uint8_t read8FlashMemoryControlRegister2(void);

/**
 * @brief Reads the value of the EBR1 register.
 * 
 * @returns The value of the register.
 */
static inline uint8_t read8EraseBlockRegister1(void);

/**
 * @brief Reads the value of the FLPWCR register.
 * 
 * @returns The value of the register.
 */
static inline uint8_t read8FlashMemoryPowerControlRegister(void);

/**
 * @brief Reads the value of the FENR register.
 * 
 * @returns The value of the register.
 */
static inline uint8_t read8FlashMemoryEnableRegister(void);

/**
 * @brief Writes the given value to the FLMCR1 register.
 * 
 * @param[in] p_value The value to write.
 */
static inline void write8FlashMemoryControlRegister1(uint8_t p_value);

/**
 * @brief Writes the given value to the EBR1 register.
 * 
 * @param[in] p_value The value to write.
 */
static inline void write8EraseBlockRegister1(uint8_t p_value);

/**
 * @brief Writes the given value to the FLPWCR register.
 * 
 * @param[in] p_value The value to write.
 */
static inline void write8FlashMemoryPowerControlRegister(uint8_t p_value);

/**
 * @brief Writes the given value to the FENR register.
 * 
 * @param[in] p_value The value to write.
 */
static inline void write8FlashMemoryEnableRegister(uint8_t p_value);

// =============================================================================
// Public function definitions
// =============================================================================
void romInit(uint8_t *p_romBuffer) {
    s_romData = p_romBuffer;
}

void romReset(void) {
    // TODO: reset registers
}

uint8_t romRead8(uint16_t p_address) {
    if((p_address & 0xc000U) != 0xc000U) {
        return s_romData[p_address];
    } else if(p_address == 0xf020U) {
        return read8FlashMemoryControlRegister1();
    } else if(p_address == 0xf021U) {
        return read8FlashMemoryControlRegister2();
    } else if(p_address == 0xf022U) {
        return read8FlashMemoryPowerControlRegister();
    } else if(p_address == 0xf023U) {
        return read8EraseBlockRegister1();
    } else if(p_address == 0xf02bU) {
        return read8FlashMemoryEnableRegister();
    } else {
        return 0xffU;
    }
}

uint16_t romRead16(uint16_t p_address) {
    uint16_t l_address = p_address & 0xfffeU;

    if((l_address & 0xc000) != 0xc000) {
        return (s_romData[l_address] << 8U) | s_romData[l_address | 0x0001U];
    } else { // TODO: registers?
        return 0xffff;
    }
}

void romWrite8(uint16_t p_address, uint8_t p_value) {
    if((p_address & 0xc000U) != 0xc000U) {
        // TODO: ROM programming
    } else if(p_address == 0xf020U) {
        write8FlashMemoryControlRegister1(p_value);
    } else if(p_address == 0xf022U) {
        write8FlashMemoryPowerControlRegister(p_value);
    } else if(p_address == 0xf023U) {
        write8EraseBlockRegister1(p_value);
    } else if(p_address == 0xf02bU) {
        write8FlashMemoryEnableRegister(p_value);
    }
}

void romWrite16(uint16_t p_address, uint8_t p_value) {
    M_UNUSED_PARAMETER(p_address);
    M_UNUSED_PARAMETER(p_value);

    // TODO
}

// =============================================================================
// Private function definitions
// =============================================================================
static inline uint8_t read8FlashMemoryControlRegister1(void) {
    // TODO: implement register
    return 0xff;
}

static inline uint8_t read8FlashMemoryControlRegister2(void) {
    // TODO: implement register
    return 0xff;
}

static inline uint8_t read8EraseBlockRegister1(void) {
    // TODO: implement register
    return 0xff;
}

static inline uint8_t read8FlashMemoryPowerControlRegister(void) {
    // TODO: implement register
    return 0xff;
}

static inline uint8_t read8FlashMemoryEnableRegister(void) {
    // TODO: implement register
    return 0xff;
}

static inline void write8FlashMemoryControlRegister1(uint8_t p_value) {
    // TODO: implement register
    M_UNUSED_PARAMETER(p_value);
}

static inline void write8EraseBlockRegister1(uint8_t p_value) {
    // TODO: implement register
    M_UNUSED_PARAMETER(p_value);
}

static inline void write8FlashMemoryPowerControlRegister(uint8_t p_value) {
    // TODO: implement register
    M_UNUSED_PARAMETER(p_value);
}

static inline void write8FlashMemoryEnableRegister(uint8_t p_value) {
    // TODO: implement register
    M_UNUSED_PARAMETER(p_value);
}
