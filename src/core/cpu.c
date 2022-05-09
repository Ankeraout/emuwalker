// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stdint.h>

#include "core/bus.h"

// =============================================================================
// Private type declarations
// =============================================================================
enum te_cpuRegister {
    E_CPUREGISTER_R0H,
    E_CPUREGISTER_R1H,
    E_CPUREGISTER_R2H,
    E_CPUREGISTER_R3H,
    E_CPUREGISTER_R4H,
    E_CPUREGISTER_R5H,
    E_CPUREGISTER_R6H,
    E_CPUREGISTER_R7H,
    E_CPUREGISTER_R0L,
    E_CPUREGISTER_R1L,
    E_CPUREGISTER_R2L,
    E_CPUREGISTER_R3L,
    E_CPUREGISTER_R4L,
    E_CPUREGISTER_R5L,
    E_CPUREGISTER_R6L,
    E_CPUREGISTER_R7L,
    E_CPUREGISTER_R0 = 0,
    E_CPUREGISTER_R1,
    E_CPUREGISTER_R2,
    E_CPUREGISTER_R3,
    E_CPUREGISTER_R4,
    E_CPUREGISTER_R5,
    E_CPUREGISTER_R6,
    E_CPUREGISTER_R7,
    E_CPUREGISTER_E0,
    E_CPUREGISTER_E1,
    E_CPUREGISTER_E2,
    E_CPUREGISTER_E3,
    E_CPUREGISTER_E4,
    E_CPUREGISTER_E5,
    E_CPUREGISTER_E6,
    E_CPUREGISTER_E7,
    E_CPUREGISTER_ER0 = 0,
    E_CPUREGISTER_ER1,
    E_CPUREGISTER_ER2,
    E_CPUREGISTER_ER3,
    E_CPUREGISTER_ER4,
    E_CPUREGISTER_ER5,
    E_CPUREGISTER_ER6,
    E_CPUREGISTER_ER7
};

union tu_cpuGeneralRegister {
    uint32_t longWord;

    struct {
        uint16_t r : 16;
        uint16_t e : 16;
    } word;

    struct {
        uint32_t rl : 8;
        uint32_t rh : 8;
        uint32_t unused : 16;
    } byte;
};

union tu_cpuFlagsRegister {
    uint8_t byte;

    struct {
        uint8_t carry : 1;
        uint8_t overflow : 1;
        uint8_t zero : 1;
        uint8_t negative : 1;
        uint8_t user1 : 1;
        uint8_t halfCarry : 1;
        uint8_t user2 : 1;
        uint8_t interruptMask : 1;
    } bitField;
};

typedef void (*tf_opcodeHandler)(void);

// =============================================================================
// Private variable declarations
// =============================================================================
static union tu_cpuFlagsRegister s_cpuFlagsRegister;
static union tu_cpuGeneralRegister s_cpuGeneralRegisters[8];
static uint32_t s_cpuRegisterPC;

/**
 * @brief This variable indicates whether the CPU has fetched the reset vector
 *        from ROM.
 */
static bool s_cpuInitialized;
static uint16_t s_opcodeBuffer[2];

// =============================================================================
// Private function declarations
// =============================================================================
/**
 * @brief Fetches a word at PC and increments PC.
 *
 * @returns The word at PC.
 */
static inline uint16_t cpuFetch16(void);

/**
 * @brief Fetches a longword at PC and increments PC.
 *
 * @returns The longword at PC.
 */
static inline uint32_t cpuFetch32(void);

/**
 * @brief Decodes an opcode.
 *
 * @returns The handler of the decoded opcode.
 */
static inline tf_opcodeHandler cpuDecode(void);

/**
 * @brief Decodes group 2 opcodes (table 2.5 in the manual)
 *
 * @returns The handler of the decoded opcode.
 */
static inline tf_opcodeHandler cpuDecodeGroup2(void);

/**
 * @brief Decodes group 3 opcodes (table 2.5 in the manual)
 *
 * @returns The handler of the decoded opcode.
 */
static inline tf_opcodeHandler cpuDecodeGroup3(void);

/**
 * @brief Gets the value of the given 8-bit CPU register.
 *
 * @param[in] p_register The register to get.
 *
 * @returns The value of the register.
 */
static inline uint8_t cpuGetRegister8(enum te_cpuRegister p_register);

/**
 * @brief Sets the value of the given 8-bit CPU register.
 *
 * @param[in] p_register The register to set.
 * @param[in] p_value The new value of the register.
 */
static inline void cpuSetRegister8(
    enum te_cpuRegister p_register,
    uint8_t p_value
);

/**
 * @brief Gets the value of the given 16-bit CPU register.
 *
 * @param[in] p_register The register to get.
 *
 * @returns The value of the register.
 */
static inline uint16_t cpuGetRegister16(enum te_cpuRegister p_register);

/**
 * @brief Sets the value of the given 16-bit CPU register.
 *
 * @param[in] p_register The register to set.
 * @param[in] p_value The new value of the register.
 */
static inline void cpuSetRegister16(
    enum te_cpuRegister p_register,
    uint16_t p_value
);

/**
 * @brief Gets the value of the given 32-bit CPU register.
 *
 * @param[in] p_register The register to get.
 *
 * @returns The value of the register.
 */
static inline uint32_t cpuGetRegister32(enum te_cpuRegister p_register);

/**
 * @brief Sets the value of the given 32-bit CPU register.
 *
 * @param[in] p_register The register to set.
 * @param[in] p_value The new value of the register.
 */
static inline void cpuSetRegister32(
    enum te_cpuRegister p_register,
    uint32_t p_value
);

/**
 * @brief Executes the ADD.B opcode.
 */
static void cpuOpcodeAddB(void);

/**
 * @brief Executes the ADD.W opcode.
 */
static void cpuOpcodeAddW(void);

/**
 * @brief Executes the ADD.L opcode.
 */
static void cpuOpcodeAddL(void);

/**
 * @brief Executes the ADDS opcode.
 */
static void cpuOpcodeAddS(void);

/**
 * @brief Executes the ADDX opcode.
 */
static void cpuOpcodeAddX(void);

/**
 * @brief Executes the AND.B opcode.
 */
static void cpuOpcodeAndB(void);

/**
 * @brief Executes the AND.W opcode.
 */
static void cpuOpcodeAndW(void);

/**
 * @brief Executes the AND.L opcode.
 */
static void cpuOpcodeAndL(void);

/**
 * @brief Executes the ANDC opcode.
 */
static void cpuOpcodeAndC(void);

/**
 * @brief Executes the BAND opcode.
 */
static void cpuOpcodeBand(void);

/**
 * @brief Executes the NOP opcode.
 */
static void cpuOpcodeNop(void);

/**
 * @brief Executes an undefined opcode.
 */
static void cpuOpcodeUndefined(void);

// =============================================================================
// Public function definitions
// =============================================================================
void cpuReset(void) {
    for(int l_registerIndex = 0; l_registerIndex < 8; l_registerIndex++) {
        s_cpuGeneralRegisters[l_registerIndex].longWord = 0x00000000U;
    }

    s_cpuFlagsRegister.byte = 0x00U;
    s_cpuFlagsRegister.bitField.interruptMask = 1;
    s_cpuRegisterPC = 0x00000000U;
    s_cpuInitialized = false;
}

void coreStep(void) {
    if(!s_cpuInitialized) {
        s_cpuRegisterPC = busRead16(0x0000U);
    }

    // Fetch
    s_opcodeBuffer[0] = cpuFetch16();

    // Decode
    tf_opcodeHandler l_opcodeHandler = cpuDecode();

    // Execute
    l_opcodeHandler();
}

// =============================================================================
// Private function definitions
// =============================================================================
static inline uint16_t cpuFetch16(void) {
    uint16_t l_returnValue = busRead16(s_cpuRegisterPC);
    s_cpuRegisterPC += 2;

    return l_returnValue;
}

static inline uint32_t cpuFetch32(void) {
    uint32_t l_returnValue =
        (busRead16(s_cpuRegisterPC) << 16)
        | busRead16(s_cpuRegisterPC + 2);

    s_cpuRegisterPC += 4;

    return l_returnValue;
}

static inline tf_opcodeHandler cpuDecode(void) {
    switch(s_opcodeBuffer[0] >> 8) {
        case 0x00: return cpuOpcodeNop;
        case 0x01: return cpuDecodeGroup2();
        case 0x02: // TODO: STC
        case 0x03: // TODO: LDC
        case 0x04: // TODO: ORG
        case 0x05: // TODO: XORG
        case 0x06: return cpuOpcodeAndC;
        case 0x07: // TODO: LDC
        case 0x08: return cpuOpcodeAddB;
        case 0x09: return cpuOpcodeAddW;
        case 0x0a: return cpuDecodeGroup2();
        case 0x0b: return cpuDecodeGroup2();
        case 0x0c:
        case 0x0d: // TODO: MOV
        case 0x0e: return cpuOpcodeAddX;
        case 0x0f: return cpuDecodeGroup2();
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13: return cpuDecodeGroup2();
        case 0x14: // TODO: OR.B
        case 0x15: // TODO: XOR.B
        case 0x16: return cpuOpcodeAndB;
        case 0x17: return cpuDecodeGroup2();
        case 0x18: // SUB.B
        case 0x19: // SUB.W
        case 0x1a:
        case 0x1b: return cpuDecodeGroup2();
        case 0x1c:
        case 0x1d: // TODO: CMP
        case 0x1e: // TODO: SUBX
        case 0x1f: return cpuDecodeGroup2();
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2a:
        case 0x2b:
        case 0x2c:
        case 0x2d:
        case 0x2e:
        case 0x2f:
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3a:
        case 0x3b:
        case 0x3c:
        case 0x3d:
        case 0x3e:
        case 0x3f: // TODO: MOV.B
        case 0x40: // TODO: BRA
        case 0x41: // TODO: BRN
        case 0x42: // TODO: BHI
        case 0x43: // TODO: BLS
        case 0x44: // TODO: BCC
        case 0x45: // TODO: BCS
        case 0x46: // TODO: BNE
        case 0x47: // TODO: BEQ
        case 0x48: // TODO: BVC
        case 0x49: // TODO: BVS
        case 0x4a: // TODO: BPL
        case 0x4b: // TODO: BMI
        case 0x4c: // TODO: BGE
        case 0x4d: // TODO: BLT
        case 0x4e: // TODO: BGT
        case 0x4f: // TODO: BLE
        case 0x50: // TODO: MULXU
        case 0x51: // TODO: DIVXU
        case 0x52: // TODO: MULXU
        case 0x53: // TODO: DIVXU
        case 0x54: // TODO: RTS
        case 0x55: // TODO: BSR
        case 0x56: // TODO: RTE
        case 0x57: // TODO: TRAPA
        case 0x58: return cpuDecodeGroup2();
        case 0x59:
        case 0x5a:
        case 0x5b: // TODO: JMP
        case 0x5c: // TODO: BSR
        case 0x5d:
        case 0x5e:
        case 0x5f: // TODO: JSR
        case 0x60: // TODO: BSET
        case 0x61: // TODO: BNOT
        case 0x62: // TODO: BCLR
        case 0x63: // TODO: BTST
        case 0x64: // TODO: OR.W
        case 0x65: // TODO: XOR.W
        case 0x66: return cpuOpcodeAndW;
        case 0x67:
            if((s_opcodeBuffer[0] & 0x0080) == 0x0000) {
                // TODO: BST
            } else {
                // TODO: BIST
            }

            break;

        case 0x68:
        case 0x69:
        case 0x6a:
        case 0x6b:
        case 0x6c:
        case 0x6d:
        case 0x6e:
        case 0x6f: // TODO: MOV
        case 0x70: // TODO: BSET
        case 0x71: // TODO: BNOT
        case 0x72: // TODO: BCLR
        case 0x73: // TODO: BTST
        case 0x74:
            if((s_opcodeBuffer[0] & 0x0080) == 0x0000) {
                // TODO: BOR
            } else {
                // TODO: BIOR
            }

            break;

        case 0x75:
            if((s_opcodeBuffer[0] & 0x0080) == 0x0000) {
                // TODO: BXOR
            } else {
                // TODO: BIXOR
            }

            break;

        case 0x76:
            if((s_opcodeBuffer[0] & 0x0080) == 0x0000) {
                return cpuOpcodeBand;
            } else {
                // TODO: BIAND
            }

            break;

        case 0x77:
            if((s_opcodeBuffer[0] & 0x0080) == 0x0000) {
                // TODO: BLD
            } else {
                // TODO: BILD
            }

            break;

        case 0x78: // TODO: MOV
        case 0x79:
        case 0x7a: return cpuDecodeGroup2();
        case 0x7b: // TODO: EEPMOV
        case 0x7c:
        case 0x7d:
        case 0x7e:
        case 0x7f: return cpuDecodeGroup3();
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8b:
        case 0x8c:
        case 0x8d:
        case 0x8e:
        case 0x8f: return cpuOpcodeAddB;
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9a:
        case 0x9b:
        case 0x9c:
        case 0x9d:
        case 0x9e:
        case 0x9f: return cpuOpcodeAddX;
        case 0xa0:
        case 0xa1:
        case 0xa2:
        case 0xa3:
        case 0xa4:
        case 0xa5:
        case 0xa6:
        case 0xa7:
        case 0xa8:
        case 0xa9:
        case 0xaa:
        case 0xab:
        case 0xac:
        case 0xad:
        case 0xae:
        case 0xaf: // TODO: CMP
        case 0xb0:
        case 0xb1:
        case 0xb2:
        case 0xb3:
        case 0xb4:
        case 0xb5:
        case 0xb6:
        case 0xb7:
        case 0xb8:
        case 0xb9:
        case 0xba:
        case 0xbb:
        case 0xbc:
        case 0xbd:
        case 0xbe:
        case 0xbf: // TODO: SUBX
        case 0xc0:
        case 0xc1:
        case 0xc2:
        case 0xc3:
        case 0xc4:
        case 0xc5:
        case 0xc6:
        case 0xc7:
        case 0xc8:
        case 0xc9:
        case 0xca:
        case 0xcb:
        case 0xcc:
        case 0xcd:
        case 0xce:
        case 0xcf: // TODO: OR
        case 0xd0:
        case 0xd1:
        case 0xd2:
        case 0xd3:
        case 0xd4:
        case 0xd5:
        case 0xd6:
        case 0xd7:
        case 0xd8:
        case 0xd9:
        case 0xda:
        case 0xdb:
        case 0xdc:
        case 0xdd:
        case 0xde:
        case 0xdf: // TODO: XOR
        case 0xe0:
        case 0xe1:
        case 0xe2:
        case 0xe3:
        case 0xe4:
        case 0xe5:
        case 0xe6:
        case 0xe7:
        case 0xe8:
        case 0xe9:
        case 0xea:
        case 0xeb:
        case 0xec:
        case 0xed:
        case 0xee:
        case 0xef: return cpuOpcodeAndB;
        case 0xf0:
        case 0xf1:
        case 0xf2:
        case 0xf3:
        case 0xf4:
        case 0xf5:
        case 0xf6:
        case 0xf7:
        case 0xf8:
        case 0xf9:
        case 0xfa:
        case 0xfb:
        case 0xfc:
        case 0xfd:
        case 0xfe:
        case 0xff: // TODO: MOV
            break;
    }
}

static inline tf_opcodeHandler cpuDecodeGroup2(void) {
    switch(s_opcodeBuffer[0] >> 4) {
        case 0x010: // TODO: MOV
        case 0x014:
            if((s_opcodeBuffer[0] & 0x0008) == 0x0000) {
                // TODO: LDC
            } else {
                // TODO: STC
            }

            break;

        case 0x018: // TODO: SLEEP
        case 0x01c:
        case 0x01d:
        case 0x01f: return cpuDecodeGroup3();
        case 0x0a0: // TODO: INC
        case 0x0a8:
        case 0x0a9:
        case 0x0aa:
        case 0x0ab:
        case 0x0ac:
        case 0x0ad:
        case 0x0ae:
        case 0x0af: return cpuOpcodeAddL;
        case 0x0b0: return cpuOpcodeAddS;
        case 0x0b5: // TODO: INC
        case 0x0b7: // TODO: INC
        case 0x0b8:
        case 0x0b9: return cpuOpcodeAddS;
        case 0x0bd:
        case 0x0bf: // TODO: INC
        case 0x0f0: // TODO: DAA
        case 0x0f8:
        case 0x0f9:
        case 0x0fa:
        case 0x0fb:
        case 0x0fc:
        case 0x0fd:
        case 0x0fe:
        case 0x0ff: // TODO: MOV
        case 0x100:
        case 0x101:
        case 0x103: // TODO: SHLL
        case 0x108:
        case 0x109:
        case 0x10b: // TODO: SHAL
        case 0x110:
        case 0x111:
        case 0x113: // TODO: SHLR
        case 0x118:
        case 0x119:
        case 0x11b: // TODO: SHAR
        case 0x120:
        case 0x121:
        case 0x123: // TODO: ROTXL
        case 0x128:
        case 0x129:
        case 0x12b: // TODO: ROTL
        case 0x130:
        case 0x131:
        case 0x133: // TODO: ROTXR
        case 0x138:
        case 0x139:
        case 0x13b: // TODO: ROTR
        case 0x170:
        case 0x171:
        case 0x173: // TODO: NOT
        case 0x175:
        case 0x177: // TODO: EXTU
        case 0x178:
        case 0x179:
        case 0x17b: // TODO: NEG
        case 0x17d:
        case 0x17f: // TODO: EXTS
        case 0x1a0: // TODO: DEC
        case 0x1a8:
        case 0x1a9:
        case 0x1aa:
        case 0x1ab:
        case 0x1ac:
        case 0x1ad:
        case 0x1ae:
        case 0x1af: // TODO: SUB
        case 0x1b0: // TODO: SUBS
        case 0x1b5:
        case 0x1b7: // TODO: DEC
        case 0x1b8:
        case 0x1b9: // TODO: SUB
        case 0x1bd:
        case 0x1bf: // TODO: DEC
        case 0x1f0: // TODO: DAS
        case 0x1f8:
        case 0x1f9:
        case 0x1fa:
        case 0x1fb:
        case 0x1fc:
        case 0x1fd:
        case 0x1fe:
        case 0x1ff: // TODO: CMP
        case 0x580: // TODO: BRA
        case 0x581: // TODO: BRN
        case 0x582: // TODO: BHI
        case 0x583: // TODO: BLS
        case 0x584: // TODO: BCC
        case 0x585: // TODO: BCS
        case 0x586: // TODO: BNE
        case 0x587: // TODO: BEQ
        case 0x588: // TODO: BVC
        case 0x589: // TODO: BVS
        case 0x58a: // TODO: BPL
        case 0x58b: // TODO: BMI
        case 0x58c: // TODO: BGE
        case 0x58d: // TODO: BLT
        case 0x58e: // TODO: BGT
        case 0x58f: // TODO: BLE
        case 0x790: // TODO: MOV
        case 0x791: return cpuOpcodeAddW;
        case 0x792: // TODO: CMP
        case 0x793: // TODO: SUB
        case 0x794: // TODO: OR
        case 0x795: // TODO: XOR
        case 0x796: return cpuOpcodeAndW;
        case 0x7a0: // TODO: MOV
        case 0x7a1: return cpuOpcodeAddL;
        case 0x7a2: // TODO: CMP
        case 0x7a3: // TODO: SUB
        case 0x7a4: // TODO: OR
        case 0x7a5: // TODO: XOR
        case 0x7a6: return cpuOpcodeAndL;

        default:
            return cpuOpcodeUndefined;
    }

    return cpuOpcodeUndefined;
}

static inline tf_opcodeHandler cpuDecodeGroup3(void) {
    s_opcodeBuffer[1] = cpuFetch16();

    switch(s_opcodeBuffer[0] >> 8) {
        case 0x01:
            if(
                ((s_opcodeBuffer[0] & 0x00ff) == 0x00c0)
                && ((s_opcodeBuffer[1] & 0xfd00) == 0x5000)
            ) {
                // TODO: MULXS
            } else if(
                ((s_opcodeBuffer[0] & 0x00ff) == 0x00d0)
                && ((s_opcodeBuffer[1] & 0xfd00) == 0x5100)
            ) {
                // TODO: DIVXS
            } else if((s_opcodeBuffer[0] & 0x00ff) == 0x00f0) {
                if((s_opcodeBuffer[1] & 0xff00) == 0x6400) {
                    // TODO: OR
                } else if((s_opcodeBuffer[1] & 0xff00) == 0x6500) {
                    // TODO: XOR
                } else if((s_opcodeBuffer[1] & 0xff00) == 0x6600) {
                    // TODO: AND
                }
            }

            break;

        case 0x7c:
            if((s_opcodeBuffer[0] & 0x000f) == 0x0000) {
                if((s_opcodeBuffer[1] & 0xff00) == 0x6300) {
                    // TODO: BTST
                } else if((s_opcodeBuffer[1] & 0xff00) == 0x7300) {
                    // TODO: BTST
                } else if((s_opcodeBuffer[1] & 0xff00) == 0x7400) {
                    if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                        // TODO: BOR
                    } else {
                        // TODO: BIOR
                    }
                } else if((s_opcodeBuffer[1] & 0xff00) == 0x7500) {
                    if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                        // TODO: BXOR
                    } else {
                        // TODO: BIXOR
                    }
                } else if((s_opcodeBuffer[1] & 0xff00) == 0x7600) {
                    if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                        return cpuOpcodeBand;
                    } else {
                        // TODO: BIAND
                    }
                } else if((s_opcodeBuffer[1] & 0xff00) == 0x7700) {
                    if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                        // TODO: BID
                    } else {
                        // TODO: BILD
                    }
                }
            }

            break;

        case 0x7d:
            if((s_opcodeBuffer[0] & 0x000f) == 0x0000) {
                if((s_opcodeBuffer[1] & 0xf000) == 0x6000) {
                    if((s_opcodeBuffer[1] & 0x0f00) == 0x0000) {
                        // TODO: BSET
                    } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0100) {
                        // TODO: BNOT
                    } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0200) {
                        // TODO: BCLR
                    } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0700) {
                        if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                            // TODO: BST
                        } else {
                            // TODO: BIST
                        }
                    }
                } else if((s_opcodeBuffer[1] & 0xf000) == 0x7000) {
                    if((s_opcodeBuffer[1] & 0x0f00) == 0x0000) {
                        // TODO: BSET
                    } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0100) {
                        // TODO: BNOT
                    } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0200) {
                        // TODO: BCLR
                    }
                }
            }

            break;

        case 0x7e:
            if((s_opcodeBuffer[1] & 0xff00) == 0x6300) {
                // TODO: BTST
            } else if((s_opcodeBuffer[1] & 0xff00) == 0x7300) {
                // TODO: BTST
            } else if((s_opcodeBuffer[1] & 0xff00) == 0x7400) {
                if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                    // TODO: BOR
                } else {
                    // TODO: BIOR
                }
            } else if((s_opcodeBuffer[1] & 0xff00) == 0x7500) {
                if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                    // TODO: BXOR
                } else {
                    // TODO: BIXOR
                }
            } else if((s_opcodeBuffer[1] & 0xff00) == 0x7600) {
                if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                   return cpuOpcodeBand;
                } else {
                    // TODO: BIAND
                }
            } else if((s_opcodeBuffer[1] & 0xff00) == 0x7700) {
                if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                    // TODO: BID
                } else {
                    // TODO: BILD
                }
            }

            break;

        case 0x7f:
            if((s_opcodeBuffer[1] & 0xf000) == 0x6000) {
                if((s_opcodeBuffer[1] & 0x0f00) == 0x0000) {
                    // TODO: BSET
                } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0100) {
                    // TODO: BNOT
                } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0200) {
                    // TODO: BCLR
                } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0700) {
                    if((s_opcodeBuffer[1] & 0x0080) == 0x0000) {
                        // TODO: BST
                    } else {
                        // TODO: BIST
                    }
                }
            } else if((s_opcodeBuffer[1] & 0xf000) == 0x7000) {
                if((s_opcodeBuffer[1] & 0x0f00) == 0x0000) {
                    // TODO: BSET
                } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0100) {
                    // TODO: BNOT
                } else if((s_opcodeBuffer[1] & 0x0f00) == 0x0200) {
                    // TODO: BCLR
                }
            }

            break;

        default:
            return cpuOpcodeUndefined;
    }

    return cpuOpcodeUndefined;
}

static inline uint8_t cpuGetRegister8(enum te_cpuRegister p_register) {
    if((p_register & 0x08U) == 0U) {
        return (uint8_t)s_cpuGeneralRegisters[p_register & 0x07U].byte.rh;
    } else {
        return (uint8_t)s_cpuGeneralRegisters[p_register & 0x07U].byte.rl;
    }
}

static inline void cpuSetRegister8(
    enum te_cpuRegister p_register,
    uint8_t p_value
) {
    if((p_register & 0x08U) == 0U) {
        s_cpuGeneralRegisters[p_register & 0x07U].byte.rh = p_value;
    } else {
        s_cpuGeneralRegisters[p_register & 0x07U].byte.rl = p_value;
    }
}

static inline uint16_t cpuGetRegister16(enum te_cpuRegister p_register) {
    if((p_register & 0x08U) == 0U) {
        return (uint8_t)s_cpuGeneralRegisters[p_register & 0x07U].word.r;
    } else {
        return (uint8_t)s_cpuGeneralRegisters[p_register & 0x07U].word.e;
    }
}

static inline void cpuSetRegister16(
    enum te_cpuRegister p_register,
    uint16_t p_value
) {
    if((p_register & 0x08U) == 0U) {
        s_cpuGeneralRegisters[p_register & 0x07U].word.r = p_value;
    } else {
        s_cpuGeneralRegisters[p_register & 0x07U].word.e = p_value;
    }
}

static inline uint32_t cpuGetRegister32(enum te_cpuRegister p_register) {
    return s_cpuGeneralRegisters[p_register].longWord;
}

static inline void cpuSetRegister32(
    enum te_cpuRegister p_register,
    uint32_t p_value
) {
    s_cpuGeneralRegisters[p_register].longWord = p_value;
}

static void cpuOpcodeAddB(void) {
    uint8_t l_operand1;
    uint8_t l_operand2;
    int l_rd;

    if((s_opcodeBuffer[0] & 0xff00) == 0x0800) { // ADD.B Rs, Rd
        l_rd = s_opcodeBuffer[0] & 0x000f;
        l_operand1 = cpuGetRegister8((s_opcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.B #xx:8, Rd
        l_rd = (s_opcodeBuffer[0] & 0x0f00) >> 8;
        l_operand1 = s_opcodeBuffer[0] & 0x00ff;
    }

    l_operand2 = cpuGetRegister8(l_rd);

    uint16_t l_result = l_operand1 + l_operand2;

    s_cpuFlagsRegister.bitField.halfCarry = (
        (
            (
                (l_operand1 & 0x0f)
                + (l_operand2 & 0x0f)
            ) & 0x10
        ) != 0
    );
    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = (
        (((l_operand1 ^ l_operand2) & 0x80) == 0)
        && (((l_operand1 ^ l_result) & 0x80) != 0)
    );
    s_cpuFlagsRegister.bitField.carry = (l_result & 0x0100) != 0;

    cpuSetRegister8(l_rd, l_result);
}

static void cpuOpcodeAddW(void) {
    uint16_t l_operand1;
    uint16_t l_operand2;
    int l_rd;

    if((s_opcodeBuffer[0] & 0xff00) == 0x0900) { // ADD.W Rs, Rd
        l_operand1 = cpuGetRegister16((s_opcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.W #xx:16, Rd
        l_operand2 = cpuFetch16();
    }

    l_rd = s_opcodeBuffer[0] & 0x000f;
    l_operand2 = cpuGetRegister16(l_rd);

    uint32_t l_result = l_operand1 + l_operand2;

    s_cpuFlagsRegister.bitField.halfCarry = (
        (
            (
                (l_operand1 & 0x0fff)
                + (l_operand2 & 0x0fff)
            ) & 0x1000
        ) != 0
    );
    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = (
        (((l_operand1 ^ l_operand2) & 0x8000) == 0)
        && (((l_operand1 ^ l_result) & 0x8000) != 0)
    );
    s_cpuFlagsRegister.bitField.carry = (l_result & 0x00010000) != 0;

    cpuSetRegister16(l_rd, l_result);
}

static void cpuOpcodeAddL(void) {
    uint32_t l_operand1;
    uint32_t l_operand2;
    int l_erd;

    if((s_opcodeBuffer[0] & 0xff00) == 0x0900) { // ADD.W Rs, Rd
        l_operand1 = cpuGetRegister32((s_opcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.L #xx:32, Rd
        l_operand1 = cpuFetch32();
    }

    l_erd = s_opcodeBuffer[0] & 0x000f;
    l_operand2 = cpuGetRegister32(l_erd);

    uint32_t l_result = l_operand1 + l_operand2;

    s_cpuFlagsRegister.bitField.halfCarry = (
        (
            (
                (l_operand1 & 0x0fffffff)
                + (l_operand2 & 0x0fffffff)
            ) & 0x10000000
        ) != 0
    );
    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = (
        (((l_operand1 ^ l_operand2) & 0x80000000) == 0)
        && (((l_operand1 ^ l_result) & 0x80000000) != 0)
    );
    s_cpuFlagsRegister.bitField.carry = l_result < l_operand1;

    cpuSetRegister32(l_erd, l_result);
}

static void cpuOpcodeAddS(void) {
    int l_erd = s_opcodeBuffer[0] & 0x0007;
    int32_t l_erdValue =
        (int32_t)((int16_t)s_cpuGeneralRegisters[l_erd].word.r);

    int32_t l_operand2;

    if((s_opcodeBuffer[0] & 0x00f0) == 0x0000) { // ADDS #1, ERd
        l_operand2 = 1;
    } else if((s_opcodeBuffer[0] & 0x00f0) == 0x0080) { // ADDS #2, ERd
        l_operand2 = 2;
    } else if((s_opcodeBuffer[0] & 0x00f0) == 0x0090) { // ADDS #4, ERd
        l_operand2 = 4;
    }

    s_cpuGeneralRegisters[l_erd].longWord = l_erdValue + l_operand2;
}

static void cpuOpcodeAddX(void) {
    int l_rd;
    uint8_t l_operand1;

    if((s_opcodeBuffer[0] & 0xff00) == 0x0e00) { // ADDX #xx:8, Rd

    } else { // ADDX Rs, Rd
        l_rd = s_opcodeBuffer[0] & 0x000f;
        l_operand1 = cpuGetRegister8((s_opcodeBuffer[0] & 0x00f0) >> 4);
    }

    uint8_t l_operand2 = cpuGetRegister8(l_rd);
    uint16_t l_carry = s_cpuFlagsRegister.bitField.carry ? 1 : 0;
    uint16_t l_result = l_operand1 + l_operand2 + l_carry;

    s_cpuFlagsRegister.bitField.halfCarry = (
        (
            (
                (l_operand1 & 0x0f)
                + (l_operand2 & 0x0f)
            ) & 0x10
        ) != 0
    );
    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = (
        (((l_operand1 ^ l_operand2) & 0x80) == 0)
        && (((l_operand1 ^ l_result) & 0x80) != 0)
    );
    s_cpuFlagsRegister.bitField.carry = (l_result & 0x0100) != 0;

    cpuSetRegister8(l_rd, l_result);
}

static void cpuOpcodeAndB(void) {
    uint8_t l_operand1;
    uint8_t l_operand2;
    int l_rd;

    if((s_opcodeBuffer[0] & 0xff00) == 0x1600) { // AND.B Rs, Rd
        l_rd = s_opcodeBuffer[0] & 0x000f;
        l_operand1 = cpuGetRegister8((s_opcodeBuffer[0] & 0x00f0) >> 4);
    } else { // AND.B #xx:8, Rd
        l_rd = (s_opcodeBuffer[0] & 0x0f00) >> 8;
        l_operand1 = s_opcodeBuffer[0] & 0x00ff;
    }

    l_operand2 = cpuGetRegister8(l_rd);

    uint16_t l_result = l_operand1 & l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister8(l_rd, l_result);
}

static void cpuOpcodeAndW(void) {
    uint16_t l_operand1;
    uint16_t l_operand2;
    int l_rd;

    if((s_opcodeBuffer[0] & 0xff00) == 0x6600) { // AND.W Rs, Rd
        l_operand1 = cpuGetRegister16((s_opcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.W #xx:16, Rd
        l_operand2 = cpuFetch16();
    }

    l_rd = s_opcodeBuffer[0] & 0x000f;
    l_operand2 = cpuGetRegister16(l_rd);

    uint32_t l_result = l_operand1 & l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister16(l_rd, l_result);
}

static void cpuOpcodeAndL(void) {
    uint32_t l_operand1;
    uint32_t l_operand2;
    int l_erd;

    if((s_opcodeBuffer[0] & 0xfff8) == 0x7a60) { // AND.L ERs, ERd
        s_opcodeBuffer[1] = cpuFetch16();

        l_operand1 = cpuGetRegister32((s_opcodeBuffer[1] & 0x0070) >> 4);
        l_erd = s_opcodeBuffer[1] & 0x0007;
    } else { // AND.L #xx:32, ERd
        l_operand1 = cpuFetch32();
        l_erd = s_opcodeBuffer[0] & 0x0007;
    }

    l_operand2 = cpuGetRegister32(l_erd);

    uint32_t l_result = l_operand1 & l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister32(l_erd, l_result);
}

static void cpuOpcodeAndC(void) {
    s_cpuFlagsRegister.byte &= s_opcodeBuffer[0];
}

static void cpuOpcodeBand(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_opcodeBuffer[0] & 0xff00) == 0x7600) { // BAND #xx:3.Rd
        l_imm = (s_opcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_opcodeBuffer[0] & 0x000f);
    } else {
        s_opcodeBuffer[1] = cpuFetch16();
        l_imm = (s_opcodeBuffer[1] & 0x0070) >> 4;

        if((s_opcodeBuffer[0] & 0xff00) == 0x7c00) { // BAND #xx:3.@ERd
            int l_erd = (s_opcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BAND #xx:3.@aa:8
            uint32_t l_address = 0xffffff00 | (s_opcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry &= (l_operand & (1 << l_imm)) != 0;
}

static void cpuOpcodeNop(void) {
    // Do nothing
}

static void cpuOpcodeUndefined(void) {
    // TODO: what happens when an undefined opcode is executed?
}
