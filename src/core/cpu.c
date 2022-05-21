// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

enum te_cpuConditionCode {
    E_CPUCONDITIONCODE_AL,
    E_CPUCONDITIONCODE_NV,
    E_CPUCONDITIONCODE_HI,
    E_CPUCONDITIONCODE_LS,
    E_CPUCONDITIONCODE_CC,
    E_CPUCONDITIONCODE_CS,
    E_CPUCONDITIONCODE_NE,
    E_CPUCONDITIONCODE_EQ,
    E_CPUCONDITIONCODE_VC,
    E_CPUCONDITIONCODE_VS,
    E_CPUCONDITIONCODE_PL,
    E_CPUCONDITIONCODE_MI,
    E_CPUCONDITIONCODE_GE,
    E_CPUCONDITIONCODE_LT,
    E_CPUCONDITIONCODE_GT,
    E_CPUCONDITIONCODE_LE,
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
static uint16_t s_cpuOpcodeBuffer[2];

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
 * @brief Checks if the given condition code is true or false.
 *
 * @param[in] p_conditionCode The condition code to check.
 *
 * @returns The value of the condition code.
 */
static inline bool cpuCheckConditionCode(
    enum te_cpuConditionCode p_conditionCode
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
 * @brief Executes the Bcc opcode (cc being a condition code).
 */
static void cpuOpcodeBcc(void);

/**
 * @brief Executes the BCLR opcode.
 */
static void cpuOpcodeBclr(void);

/**
 * @brief Executes the BIAND opcode.
 */
static void cpuOpcodeBiand(void);

/**
 * @brief Executes the BILD opcode.
 */
static void cpuOpcodeBild(void);

/**
 * @brief Executes the BIOR opcode.
 */
static void cpuOpcodeBior(void);

/**
 * @brief Executes the BIST opcode.
 */
static void cpuOpcodeBist(void);

/**
 * @brief Executes the BIXOR opcode.
 */
static void cpuOpcodeBixor(void);

/**
 * @brief Executes the BLD opcode.
 */
static void cpuOpcodeBld(void);

/**
 * @brief Executes the BNOT opcode.
 */
static void cpuOpcodeBnot(void);

/**
 * @brief Executes the BOR opcode.
 */
static void cpuOpcodeBor(void);

/**
 * @brief Executes the BSET opcode.
 */
static void cpuOpcodeBset(void);

/**
 * @brief Executes the BSR opcode.
 */
static void cpuOpcodeBsr(void);

/**
 * @brief Executes the BST opcode.
 */
static void cpuOpcodeBst(void);

/**
 * @brief Executes the BTST opcode.
 */
static void cpuOpcodeBtst(void);

/**
 * @brief Executes the BXOR opcode.
 */
static void cpuOpcodeBxor(void);

/**
 * @brief Executes the CMP.B opcode.
 */
static void cpuOpcodeCmpB(void);

/**
 * @brief Executes the CMP.W opcode.
 */
static void cpuOpcodeCmpW(void);

/**
 * @brief Executes the CMP.L opcode.
 */
static void cpuOpcodeCmpL(void);

/**
 * @brief Executes the DAA opcode.
 */
static void cpuOpcodeDaa(void);

/**
 * @brief Executes the DAS opcode.
 */
static void cpuOpcodeDas(void);

/**
 * @brief Executes the DEC.B opcode.
 */
static void cpuOpcodeDecB(void);

/**
 * @brief Executes the DEC.W opcode.
 */
static void cpuOpcodeDecW(void);

/**
 * @brief Executes the DEC.L opcode.
 */
static void cpuOpcodeDecL(void);

/**
 * @brief Executes the DIVXS.B opcode.
 */
static void cpuOpcodeDivxsB(void);

/**
 * @brief Executes the DIVXS.W opcode.
 */
static void cpuOpcodeDivxsW(void);

/**
 * @brief Executes the DIVXU.B opcode.
 */
static void cpuOpcodeDivxuB(void);

/**
 * @brief Executes the DIVXU.W opcode.
 */
static void cpuOpcodeDivxuW(void);

/**
 * @brief Executes the EEPMOV.B opcode.
 */
static void cpuOpcodeEepmovB(void);

/**
 * @brief Executes the EEPMOV.W opcode.
 */
static void cpuOpcodeEepmovW(void);

/**
 * @brief Executes the EXTS.W opcode.
 */
static void cpuOpcodeExtsW(void);

/**
 * @brief Executes the EXTS.L opcode.
 */
static void cpuOpcodeExtsL(void);

/**
 * @brief Executes the EXTU.W opcode.
 */
static void cpuOpcodeExtuW(void);

/**
 * @brief Executes the EXTU.L opcode.
 */
static void cpuOpcodeExtuL(void);

/**
 * @brief Executes the INC.B opcode.
 */
static void cpuOpcodeIncB(void);

/**
 * @brief Executes the INC.W opcode.
 */
static void cpuOpcodeIncW(void);

/**
 * @brief Executes the INC.L opcode.
 */
static void cpuOpcodeIncL(void);

/**
 * @brief Executes the JMP opcode.
 */
static void cpuOpcodeJmp(void);

/**
 * @brief Executes the JSR opcode.
 */
static void cpuOpcodeJsr(void);

/**
 * @brief Executes the LDC.B opcode.
 */
static void cpuOpcodeLdcB(void);

/**
 * @brief Executes the LDC.W opcode.
 */
static void cpuOpcodeLdcW(void);

/**
 * @brief Executes the MOV.B Rs, Rd opcode.
 */
static void cpuOpcodeMovB1(void);

/**
 * @brief Executes the MOV.W Rs, Rd opcode.
 */
static void cpuOpcodeMovW1(void);

/**
 * @brief Executes the MOV.L ERs, ERd opcode.
 */
static void cpuOpcodeMovL1(void);

/**
 * @brief Executes the MOV.B (EAs), Rd opcode.
 */
static void cpuOpcodeMovB2(void);

/**
 * @brief Executes the MOV.W (EAs), Rd opcode.
 */
static void cpuOpcodeMovW2(void);

/**
 * @brief Executes the MOV.L (EAs), ERd opcode.
 */
static void cpuOpcodeMovL2(void);

/**
 * @brief Executes the MOV.B Rs, (EAd) opcode.
 */
static void cpuOpcodeMovB3(void);

/**
 * @brief Executes the MOV.W Rs, (EAd) opcode.
 */
static void cpuOpcodeMovW3(void);

/**
 * @brief Executes the MOV.L ERs, (EAd) opcode.
 */
static void cpuOpcodeMovL3(void);

/**
 * @brief Executes the MOVFPE opcode.
 */
static void cpuOpcodeMovfpe(void);

/**
 * @brief Executes the MOVTPE opcode.
 */
static void cpuOpcodeMovtpe(void);

/**
 * @brief Executes the MULXS.B opcode.
 */
static void cpuOpcodeMulxsB(void);

/**
 * @brief Executes the MULXS.W opcode.
 */
static void cpuOpcodeMulxsW(void);

/**
 * @brief Executes the MULXU.B opcode.
 */
static void cpuOpcodeMulxuB(void);

/**
 * @brief Executes the MULXU.W opcode.
 */
static void cpuOpcodeMulxuW(void);

/**
 * @brief Executes the NEG.B opcode.
 */
static void cpuOpcodeNegB(void);

/**
 * @brief Executes the NEG.W opcode.
 */
static void cpuOpcodeNegW(void);

/**
 * @brief Executes the NEG.L opcode.
 */
static void cpuOpcodeNegL(void);

/**
 * @brief Executes the NOP opcode.
 */
static void cpuOpcodeNop(void);

/**
 * @brief Executes the NOT.B opcode.
 */
static void cpuOpcodeNotB(void);

/**
 * @brief Executes the NOT.W opcode.
 */
static void cpuOpcodeNotW(void);

/**
 * @brief Executes the NOT.L opcode.
 */
static void cpuOpcodeNotL(void);

/**
 * @brief Executes the OR.B opcode.
 */
static void cpuOpcodeOrB(void);

/**
 * @brief Executes the OR.W opcode.
 */
static void cpuOpcodeOrW(void);

/**
 * @brief Executes the OR.L opcode.
 */
static void cpuOpcodeOrL(void);

/**
 * @brief Executes the ORC opcode.
 */
static void cpuOpcodeOrc(void);

/**
 * @brief Executes the ROTL.B opcode.
 */
static void cpuOpcodeRotlB(void);

/**
 * @brief Executes the ROTL.W opcode.
 */
static void cpuOpcodeRotlW(void);

/**
 * @brief Executes the ROTL.L opcode.
 */
static void cpuOpcodeRotlL(void);

/**
 * @brief Executes the ROTR.B opcode.
 */
static void cpuOpcodeRotrB(void);

/**
 * @brief Executes the ROTR.W opcode.
 */
static void cpuOpcodeRotrW(void);

/**
 * @brief Executes the ROTR.L opcode.
 */
static void cpuOpcodeRotrL(void);

/**
 * @brief Executes the ROTXL.B opcode.
 */
static void cpuOpcodeRotxlB(void);

/**
 * @brief Executes the ROTXL.W opcode.
 */
static void cpuOpcodeRotxlW(void);

/**
 * @brief Executes the ROTXL.L opcode.
 */
static void cpuOpcodeRotxlL(void);

/**
 * @brief Executes the ROTXR.B opcode.
 */
static void cpuOpcodeRotxrB(void);

/**
 * @brief Executes the ROTXR.W opcode.
 */
static void cpuOpcodeRotxrW(void);

/**
 * @brief Executes the ROTXR.L opcode.
 */
static void cpuOpcodeRotxrL(void);

/**
 * @brief Executes the RTE opcode.
 */
static void cpuOpcodeRte(void);

/**
 * @brief Executes the RTS opcode.
 */
static void cpuOpcodeRts(void);

/**
 * @brief Executes the SHAL.B opcode.
 */
static void cpuOpcodeShalB(void);

/**
 * @brief Executes the SHAL.W opcode.
 */
static void cpuOpcodeShalW(void);

/**
 * @brief Executes the SHAL.L opcode.
 */
static void cpuOpcodeShalL(void);

/**
 * @brief Executes the SHAR.B opcode.
 */
static void cpuOpcodeSharB(void);

/**
 * @brief Executes the SHAR.W opcode.
 */
static void cpuOpcodeSharW(void);

/**
 * @brief Executes the SHAR.L opcode.
 */
static void cpuOpcodeSharL(void);

/**
 * @brief Executes the SHLL.B opcode.
 */
static void cpuOpcodeShllB(void);

/**
 * @brief Executes the SHLL.W opcode.
 */
static void cpuOpcodeShllW(void);

/**
 * @brief Executes the SHLL.L opcode.
 */
static void cpuOpcodeShllL(void);

/**
 * @brief Executes the SHLR.B opcode.
 */
static void cpuOpcodeShlrB(void);

/**
 * @brief Executes the SHLR.W opcode.
 */
static void cpuOpcodeShlrW(void);

/**
 * @brief Executes the SHLR.L opcode.
 */
static void cpuOpcodeShlrL(void);

/**
 * @brief Executes the SLEEP opcode.
 */
static void cpuOpcodeSleep(void);

/**
 * @brief Executes the STC.B opcode.
 */
static void cpuOpcodeStcB(void);

/**
 * @brief Executes the STC.W opcode.
 */
static void cpuOpcodeStcW(void);

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
        s_cpuInitialized = true;
    }

    // Trace
    printf("===========================================================\n");
    printf(
        "ER0=0x%08x ER1=0x%08x ER2=0x%08x ER3=0x%08x\n",
        cpuGetRegister32(E_CPUREGISTER_ER0),
        cpuGetRegister32(E_CPUREGISTER_ER1),
        cpuGetRegister32(E_CPUREGISTER_ER2),
        cpuGetRegister32(E_CPUREGISTER_ER3)
    );
    printf(
        "ER4=0x%08x ER5=0x%08x ER6=0x%08x ER7=0x%08x\n",
        cpuGetRegister32(E_CPUREGISTER_ER4),
        cpuGetRegister32(E_CPUREGISTER_ER5),
        cpuGetRegister32(E_CPUREGISTER_ER6),
        cpuGetRegister32(E_CPUREGISTER_ER7)
    );
    printf(
        "PC=0x%08x FLAGS=0x%02x\n",
        s_cpuRegisterPC,
        s_cpuFlagsRegister.byte
    );

    // Fetch
    s_cpuOpcodeBuffer[0] = cpuFetch16();

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
    switch(s_cpuOpcodeBuffer[0] >> 8) {
        case 0x00: return cpuOpcodeNop;
        case 0x01: return cpuDecodeGroup2();
        case 0x02: return cpuOpcodeStcB;
        case 0x03: return cpuOpcodeLdcB;
        case 0x04: return cpuOpcodeOrc;
        case 0x05: // TODO: XORG
        case 0x06: return cpuOpcodeAndC;
        case 0x07: return cpuOpcodeLdcB;
        case 0x08: return cpuOpcodeAddB;
        case 0x09: return cpuOpcodeAddW;
        case 0x0a: return cpuDecodeGroup2();
        case 0x0b: return cpuDecodeGroup2();
        case 0x0c: return cpuOpcodeMovB1;
        case 0x0d: return cpuOpcodeMovW1;
        case 0x0e: return cpuOpcodeAddX;
        case 0x0f: return cpuDecodeGroup2();
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13: return cpuDecodeGroup2();
        case 0x14: return cpuOpcodeOrB;
        case 0x15: // TODO: XOR.B
        case 0x16: return cpuOpcodeAndB;
        case 0x17: return cpuDecodeGroup2();
        case 0x18: // SUB.B
        case 0x19: // SUB.W
        case 0x1a:
        case 0x1b: return cpuDecodeGroup2();
        case 0x1c: return cpuOpcodeCmpB;
        case 0x1d: return cpuOpcodeCmpW;
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
        case 0x2f: return cpuOpcodeMovB2;
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
        case 0x3f: return cpuOpcodeMovB3;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4c:
        case 0x4d:
        case 0x4e:
        case 0x4f: return cpuOpcodeBcc;
        case 0x50: return cpuOpcodeMulxuB;
        case 0x51: return cpuOpcodeDivxuB;
        case 0x52: return cpuOpcodeMulxuW;
        case 0x53: return cpuOpcodeDivxuW;
        case 0x54: return cpuOpcodeRts;
        case 0x55: return cpuOpcodeBsr;
        case 0x56: return cpuOpcodeRte;
        case 0x57: // TODO: TRAPA
        case 0x58: return cpuDecodeGroup2();
        case 0x59:
        case 0x5a:
        case 0x5b: return cpuOpcodeJmp;
        case 0x5c: return cpuOpcodeBsr;
        case 0x5d:
        case 0x5e:
        case 0x5f: return cpuOpcodeJsr;
        case 0x60: return cpuOpcodeBset;
        case 0x61: return cpuOpcodeBnot;
        case 0x62: return cpuOpcodeBclr;
        case 0x63: return cpuOpcodeBtst;
        case 0x64: return cpuOpcodeOrW;
        case 0x65: // TODO: XOR.W
        case 0x66: return cpuOpcodeAndW;
        case 0x67:
            if((s_cpuOpcodeBuffer[0] & 0x0080) == 0x0000) {
                return cpuOpcodeBst;
            } else {
                return cpuOpcodeBist;
            }

            break;

        case 0x68:
        case 0x6c:
        case 0x6e:
            if((s_cpuOpcodeBuffer[0] & 0x0080) == 0x0000) { // MOV.B (EAs), ERd
                return cpuOpcodeMovB2;
            } else { // MOV.B Rs, (EAd)
                return cpuOpcodeMovB3;
            }

        case 0x6a:
            if((s_cpuOpcodeBuffer[0] & 0x00c0) == 0x0000) { // MOV.B (EAs), ERd
                return cpuOpcodeMovB2;
            } else if((s_cpuOpcodeBuffer[0] & 0x00c0) == 0x0040) { // MOVFPE
                                                                   // @aa:16, Rd
                return cpuOpcodeMovfpe;
            } else if((s_cpuOpcodeBuffer[0] & 0x00c0) == 0x0080) { // MOV.B
                                                                   // (EAs), ERd
                return cpuOpcodeMovB3;
            } else { // MOVTPE @aa:16, Rd
                return cpuOpcodeMovtpe;
            }

        case 0x69:
        case 0x6b:
        case 0x6d:
        case 0x6f:
            if((s_cpuOpcodeBuffer[0] & 0x0080) == 0x0000) { // MOV.W (EAs), ERd
                return cpuOpcodeMovW2;
            } else { // MOV.W Rs, (EAd)
                return cpuOpcodeMovW3;
            }

        return cpuOpcodeMovW2;
        case 0x70: return cpuOpcodeBset;
        case 0x71: return cpuOpcodeBnot;
        case 0x72: return cpuOpcodeBclr;
        case 0x73: return cpuOpcodeBtst;
        case 0x74:
            if((s_cpuOpcodeBuffer[0] & 0x0080) == 0x0000) {
                return cpuOpcodeBor;
            } else {
                return cpuOpcodeBior;
            }

            break;

        case 0x75:
            if((s_cpuOpcodeBuffer[0] & 0x0080) == 0x0000) {
                return cpuOpcodeBxor;
            } else {
                return cpuOpcodeBixor;
            }

            break;

        case 0x76:
            if((s_cpuOpcodeBuffer[0] & 0x0080) == 0x0000) {
                return cpuOpcodeBand;
            } else {
                return cpuOpcodeBiand;
            }

            break;

        case 0x77:
            if((s_cpuOpcodeBuffer[0] & 0x0080) == 0x0000) {
                return cpuOpcodeBld;
            } else {
                return cpuOpcodeBild;
            }

            break;

        case 0x78:
            s_cpuOpcodeBuffer[1] = cpuFetch16();

            if((s_cpuOpcodeBuffer[1] & 0xfff0) == 0x6a20) { // MOV.B (EAs), Rd
                return cpuOpcodeMovB2;
            } else if((s_cpuOpcodeBuffer[1] & 0xfff0) == 0x6aa0) { // MOV.B Rd,
                                                                 // (EAs)
                return cpuOpcodeMovB3;
            } else if((s_cpuOpcodeBuffer[1] & 0xfff0) == 0x6b20) { // MOV.W
                                                                   // (EAs), Rd
                return cpuOpcodeMovW2;
            } else { // MOV.W Rd, (EAs)
                return cpuOpcodeMovW3;
            }
        case 0x79:
        case 0x7a: return cpuDecodeGroup2();
        case 0x7b:
            cpuFetch16(); // We don't care about the 3rd and 4th bytes.

            if(s_cpuOpcodeBuffer[0] == 0x7b5c) {
                return cpuOpcodeEepmovB;
            } else {
                return cpuOpcodeEepmovW;
            }
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
        case 0xaf: return cpuOpcodeCmpB;
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
        case 0xcf: return cpuOpcodeOrB;
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
        case 0xff: return cpuOpcodeMovB2;
            break;
    }
}

static inline tf_opcodeHandler cpuDecodeGroup2(void) {
    switch(s_cpuOpcodeBuffer[0] >> 4) {
        case 0x010:
            s_cpuOpcodeBuffer[1] = cpuFetch16();

            if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) { // MOV.L (EAs), ERd
                return cpuOpcodeMovL2;
            } else { // MOV.L ERd, (EAs)
                return cpuOpcodeMovL3;
            }
        case 0x014:
            s_cpuOpcodeBuffer[1] = cpuFetch16();

            if((s_cpuOpcodeBuffer[1] & 0x0008) == 0x0000) {
                return cpuOpcodeLdcW;
            } else {
                return cpuOpcodeStcW;
            }

            break;

        case 0x018: return cpuOpcodeSleep;
        case 0x01c:
        case 0x01d:
        case 0x01f: return cpuDecodeGroup3();
        case 0x0a0: return cpuOpcodeIncB;
        case 0x0a8:
        case 0x0a9:
        case 0x0aa:
        case 0x0ab:
        case 0x0ac:
        case 0x0ad:
        case 0x0ae:
        case 0x0af: return cpuOpcodeAddL;
        case 0x0b0: return cpuOpcodeAddS;
        case 0x0b5: return cpuOpcodeIncW;
        case 0x0b7: return cpuOpcodeIncL;
        case 0x0b8:
        case 0x0b9: return cpuOpcodeAddS;
        case 0x0bd: return cpuOpcodeIncW;
        case 0x0bf: return cpuOpcodeIncL;
        case 0x0f0: return cpuOpcodeDaa;
        case 0x0f8:
        case 0x0f9:
        case 0x0fa:
        case 0x0fb:
        case 0x0fc:
        case 0x0fd:
        case 0x0fe:
        case 0x0ff: return cpuOpcodeMovL1;
        case 0x100: return cpuOpcodeShllB;
        case 0x101: return cpuOpcodeShllW;
        case 0x103: return cpuOpcodeShllL;
        case 0x108: return cpuOpcodeShalB;
        case 0x109: return cpuOpcodeShalW;
        case 0x10b: return cpuOpcodeShalL;
        case 0x110: return cpuOpcodeShlrB;
        case 0x111: return cpuOpcodeShlrW;
        case 0x113: return cpuOpcodeShlrL;
        case 0x118: return cpuOpcodeSharB;
        case 0x119: return cpuOpcodeSharW;
        case 0x11b: return cpuOpcodeSharL;
        case 0x120: return cpuOpcodeRotxlB;
        case 0x121: return cpuOpcodeRotxlW;
        case 0x123: return cpuOpcodeRotxlL;
        case 0x128: return cpuOpcodeRotlB;
        case 0x129: return cpuOpcodeRotlW;
        case 0x12b: return cpuOpcodeRotlL;
        case 0x130: return cpuOpcodeRotxrB;
        case 0x131: return cpuOpcodeRotxrW;
        case 0x133: return cpuOpcodeRotxrL;
        case 0x138: return cpuOpcodeRotrB;
        case 0x139: return cpuOpcodeRotrW;
        case 0x13b: return cpuOpcodeRotrL;
        case 0x170: return cpuOpcodeNotB;
        case 0x171: return cpuOpcodeNotW;
        case 0x173: return cpuOpcodeNotL;
        case 0x175: return cpuOpcodeExtuW;
        case 0x177: return cpuOpcodeExtuL;
        case 0x178: return cpuOpcodeNegB;
        case 0x179: return cpuOpcodeNegW;
        case 0x17b: return cpuOpcodeNegL;
        case 0x17d: return cpuOpcodeExtsW;
        case 0x17f: return cpuOpcodeExtsL;
        case 0x1a0: return cpuOpcodeDecB;
        case 0x1a8:
        case 0x1a9:
        case 0x1aa:
        case 0x1ab:
        case 0x1ac:
        case 0x1ad:
        case 0x1ae:
        case 0x1af: // TODO: SUB
        case 0x1b0: // TODO: SUBS
        case 0x1b5: return cpuOpcodeDecW;
        case 0x1b7: return cpuOpcodeDecL;
        case 0x1b8:
        case 0x1b9: // TODO: SUB
        case 0x1bd: return cpuOpcodeDecW;
        case 0x1bf: return cpuOpcodeDecL;
        case 0x1f0: return cpuOpcodeDas;
        case 0x1f8:
        case 0x1f9:
        case 0x1fa:
        case 0x1fb:
        case 0x1fc:
        case 0x1fd:
        case 0x1fe:
        case 0x1ff: return cpuOpcodeCmpL;
        case 0x580:
        case 0x581:
        case 0x582:
        case 0x583:
        case 0x584:
        case 0x585:
        case 0x586:
        case 0x587:
        case 0x588:
        case 0x589:
        case 0x58a:
        case 0x58b:
        case 0x58c:
        case 0x58d:
        case 0x58e:
        case 0x58f: return cpuOpcodeBcc;
        case 0x790: return cpuOpcodeMovW2;
        case 0x791: return cpuOpcodeAddW;
        case 0x792: return cpuOpcodeCmpW;
        case 0x793: // TODO: SUB
        case 0x794: return cpuOpcodeOrW;
        case 0x795: // TODO: XOR
        case 0x796: return cpuOpcodeAndW;
        case 0x7a0: return cpuOpcodeMovL2;
        case 0x7a1: return cpuOpcodeAddL;
        case 0x7a2: return cpuOpcodeCmpL;
        case 0x7a3: // TODO: SUB
        case 0x7a4: return cpuOpcodeOrL;
        case 0x7a5: // TODO: XOR
        case 0x7a6: return cpuOpcodeAndL;

        default:
            return cpuOpcodeUndefined;
    }

    return cpuOpcodeUndefined;
}

static inline tf_opcodeHandler cpuDecodeGroup3(void) {
    s_cpuOpcodeBuffer[1] = cpuFetch16();

    switch(s_cpuOpcodeBuffer[0] >> 8) {
        case 0x01:
            if(
                ((s_cpuOpcodeBuffer[0] & 0x00ff) == 0x00c0)
                && ((s_cpuOpcodeBuffer[1] & 0xfd00) == 0x5000)
            ) {
                if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x5000) { // MULXS.B Rs,
                                                                // Rd
                    return cpuOpcodeMulxsB;
                } else { // MULXS.W Rs, Rd
                    return cpuOpcodeMulxsW;
                }
            } else if(
                ((s_cpuOpcodeBuffer[0] & 0x00ff) == 0x00d0)
                && ((s_cpuOpcodeBuffer[1] & 0xfd00) == 0x5100)
            ) {
                if((s_cpuOpcodeBuffer[1] & 0x0200) == 0x0000) {
                    return cpuOpcodeDivxsB;
                } else {
                    return cpuOpcodeDivxsW;
                }
            } else if((s_cpuOpcodeBuffer[0] & 0x00ff) == 0x00f0) {
                if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6400) {
                    return cpuOpcodeOrL;
                } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6500) {
                    // TODO: XOR
                } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6600) {
                    // TODO: AND
                }
            }

            break;

        case 0x7c:
            if((s_cpuOpcodeBuffer[0] & 0x000f) == 0x0000) {
                if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6300) {
                    return cpuOpcodeBtst;
                } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7300) {
                    return cpuOpcodeBtst;
                } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7400) {
                    if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                        return cpuOpcodeBor;
                    } else {
                        return cpuOpcodeBior;
                    }
                } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7500) {
                    if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                        return cpuOpcodeBxor;
                    } else {
                        return cpuOpcodeBixor;
                    }
                } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7600) {
                    if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                        return cpuOpcodeBand;
                    } else {
                        return cpuOpcodeBiand;
                    }
                } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7700) {
                    if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                        // TODO: BID
                    } else {
                        return cpuOpcodeBild;
                    }
                }
            }

            break;

        case 0x7d:
            if((s_cpuOpcodeBuffer[0] & 0x000f) == 0x0000) {
                if((s_cpuOpcodeBuffer[1] & 0xf000) == 0x6000) {
                    if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0000) {
                        return cpuOpcodeBset;
                    } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0100) {
                        return cpuOpcodeBnot;
                    } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0200) {
                        return cpuOpcodeBclr;
                    } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0700) {
                        if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                            return cpuOpcodeBst;
                        } else {
                            return cpuOpcodeBist;
                        }
                    }
                } else if((s_cpuOpcodeBuffer[1] & 0xf000) == 0x7000) {
                    if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0000) {
                        return cpuOpcodeBset;
                    } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0100) {
                        return cpuOpcodeBnot;
                    } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0200) {
                        return cpuOpcodeBclr;
                    }
                }
            }

            break;

        case 0x7e:
            if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6300) {
                return cpuOpcodeBtst;
            } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7300) {
                return cpuOpcodeBtst;
            } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7400) {
                if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                    return cpuOpcodeBor;
                } else {
                    return cpuOpcodeBior;
                }
            } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7500) {
                if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                    return cpuOpcodeBxor;
                } else {
                    return cpuOpcodeBixor;
                }
            } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7600) {
                if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                   return cpuOpcodeBand;
                } else {
                    return cpuOpcodeBiand;
                }
            } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7700) {
                if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                    // TODO: BID
                } else {
                    return cpuOpcodeBild;
                }
            }

            break;

        case 0x7f:
            if((s_cpuOpcodeBuffer[1] & 0xf000) == 0x6000) {
                if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0000) {
                    return cpuOpcodeBset;
                } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0100) {
                    return cpuOpcodeBnot;
                } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0200) {
                    return cpuOpcodeBclr;
                } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0700) {
                    if((s_cpuOpcodeBuffer[1] & 0x0080) == 0x0000) {
                        return cpuOpcodeBst;
                    } else {
                        return cpuOpcodeBist;
                    }
                }
            } else if((s_cpuOpcodeBuffer[1] & 0xf000) == 0x7000) {
                if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0000) {
                    return cpuOpcodeBset;
                } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0100) {
                    return cpuOpcodeBnot;
                } else if((s_cpuOpcodeBuffer[1] & 0x0f00) == 0x0200) {
                    return cpuOpcodeBclr;
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

static inline bool cpuCheckConditionCode(
    enum te_cpuConditionCode p_conditionCode
) {
    switch(p_conditionCode) {
        case E_CPUCONDITIONCODE_AL: return true;
        case E_CPUCONDITIONCODE_HI:
            return !(
                s_cpuFlagsRegister.bitField.carry
                | s_cpuFlagsRegister.bitField.zero
            );

        case E_CPUCONDITIONCODE_LS:
            return s_cpuFlagsRegister.bitField.carry
                | s_cpuFlagsRegister.bitField.zero;

        case E_CPUCONDITIONCODE_CC: return !s_cpuFlagsRegister.bitField.carry;
        case E_CPUCONDITIONCODE_CS: return s_cpuFlagsRegister.bitField.carry;
        case E_CPUCONDITIONCODE_NE: return !s_cpuFlagsRegister.bitField.zero;
        case E_CPUCONDITIONCODE_EQ: return s_cpuFlagsRegister.bitField.zero;
        case E_CPUCONDITIONCODE_VC:
            return !s_cpuFlagsRegister.bitField.overflow;

        case E_CPUCONDITIONCODE_VS: return s_cpuFlagsRegister.bitField.overflow;
        case E_CPUCONDITIONCODE_PL:
            return !s_cpuFlagsRegister.bitField.negative;

        case E_CPUCONDITIONCODE_MI: return s_cpuFlagsRegister.bitField.negative;
        case E_CPUCONDITIONCODE_GE:
            return !(
                s_cpuFlagsRegister.bitField.negative
                ^ s_cpuFlagsRegister.bitField.overflow
            );

        case E_CPUCONDITIONCODE_LT:
            return s_cpuFlagsRegister.bitField.negative
                ^ s_cpuFlagsRegister.bitField.overflow;

        case E_CPUCONDITIONCODE_GT:
            return !(
                s_cpuFlagsRegister.bitField.zero
                | (
                    s_cpuFlagsRegister.bitField.negative
                    ^ s_cpuFlagsRegister.bitField.overflow
                )
            );

        case E_CPUCONDITIONCODE_LE:
            return
                s_cpuFlagsRegister.bitField.zero
                | (
                    s_cpuFlagsRegister.bitField.negative
                    ^ s_cpuFlagsRegister.bitField.overflow
                );

        default:
            return false;
    }
}

static void cpuOpcodeAddB(void) {
    uint8_t l_operand1;
    uint8_t l_operand2;
    int l_rd;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x0800) { // ADD.B Rs, Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand1 = cpuGetRegister8((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.B #xx:8, Rd
        l_rd = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 8;
        l_operand1 = s_cpuOpcodeBuffer[0] & 0x00ff;
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

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x0900) { // ADD.W Rs, Rd
        l_operand1 = cpuGetRegister16((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.W #xx:16, Rd
        l_operand2 = cpuFetch16();
    }

    l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
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

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x0900) { // ADD.W Rs, Rd
        l_operand1 = cpuGetRegister32((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.L #xx:32, Rd
        l_operand1 = cpuFetch32();
    }

    l_erd = s_cpuOpcodeBuffer[0] & 0x000f;
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
    int l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    int32_t l_erdValue =
        (int32_t)((int16_t)s_cpuGeneralRegisters[l_erd].word.r);

    int32_t l_operand2;

    if((s_cpuOpcodeBuffer[0] & 0x00f0) == 0x0000) { // ADDS #1, ERd
        l_operand2 = 1;
    } else if((s_cpuOpcodeBuffer[0] & 0x00f0) == 0x0080) { // ADDS #2, ERd
        l_operand2 = 2;
    } else if((s_cpuOpcodeBuffer[0] & 0x00f0) == 0x0090) { // ADDS #4, ERd
        l_operand2 = 4;
    }

    s_cpuGeneralRegisters[l_erd].longWord = l_erdValue + l_operand2;
}

static void cpuOpcodeAddX(void) {
    int l_rd;
    uint8_t l_operand1;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x0e00) { // ADDX #xx:8, Rd

    } else { // ADDX Rs, Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand1 = cpuGetRegister8((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
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

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x1600) { // AND.B Rs, Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand1 = cpuGetRegister8((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
    } else { // AND.B #xx:8, Rd
        l_rd = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 8;
        l_operand1 = s_cpuOpcodeBuffer[0] & 0x00ff;
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

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6600) { // AND.W Rs, Rd
        l_operand1 = cpuGetRegister16((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
    } else { // ADD.W #xx:16, Rd
        l_operand2 = cpuFetch16();
    }

    l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
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

    if((s_cpuOpcodeBuffer[0] & 0xfff8) == 0x7a60) { // AND.L ERs, ERd
        s_cpuOpcodeBuffer[1] = cpuFetch16();

        l_operand1 = cpuGetRegister32((s_cpuOpcodeBuffer[1] & 0x0070) >> 4);
        l_erd = s_cpuOpcodeBuffer[1] & 0x0007;
    } else { // AND.L #xx:32, ERd
        l_operand1 = cpuFetch32();
        l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    }

    l_operand2 = cpuGetRegister32(l_erd);

    uint32_t l_result = l_operand1 & l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister32(l_erd, l_result);
}

static void cpuOpcodeAndC(void) {
    s_cpuFlagsRegister.byte &= s_cpuOpcodeBuffer[0];
}

static void cpuOpcodeBand(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7600) { // BAND #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BAND #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BAND #xx:3, @aa:8
            uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry &= (l_operand & (1 << l_imm)) != 0;
}

static void cpuOpcodeBcc(void) {
    enum te_cpuConditionCode l_conditionCode;
    int16_t l_disp;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x5800) {
        l_disp = cpuFetch16();
        l_conditionCode = (s_cpuOpcodeBuffer[0] & 0x00f0) >> 4;
    } else {
        l_disp = (int16_t)((int8_t)s_cpuOpcodeBuffer[0]);
        l_conditionCode = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 8;
    }

    if(cpuCheckConditionCode(l_conditionCode)) {
        s_cpuRegisterPC += l_disp;
    }
}

static void cpuOpcodeBclr(void) {
    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6200) { // BCLR Rn, Rd
        int l_rn = (s_cpuOpcodeBuffer[0] & 0x00f0) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        int l_mask = ~(1 << cpuGetRegister8(l_rn));

        cpuSetRegister8(l_rd, cpuGetRegister8(l_rd) & l_mask);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7200) { // BCLR #xx:3, Rd
        int l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        int l_mask = ~(1 << l_imm);

        cpuSetRegister8(l_rd, cpuGetRegister8(l_rd) & l_mask);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();

        if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6200) {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7d00) { // BCLR Rn, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
                int l_mask = ~(1 << cpuGetRegister8(l_rn));

                busWrite8(
                    cpuGetRegister32(l_erd),
                    busRead8(cpuGetRegister32(l_erd)) & l_mask
                );
            } else { // BCLR Rn, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
                int l_mask = ~(1 << cpuGetRegister8(l_rn));

                busWrite8(l_abs, busRead8(l_abs) & l_mask);
            }
        } else {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7d00) { // BCLR #xx:3, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
                int l_mask = ~(1 << l_imm);

                busWrite8(
                    cpuGetRegister32(l_erd),
                    busRead8(cpuGetRegister32(l_erd)) & l_mask
                );
            } else { // BCLR #xx:3, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
                int l_mask = ~(1 << l_imm);

                busWrite8(l_abs, busRead8(l_abs) & l_mask);
            }
        }
    }
}

static void cpuOpcodeBiand(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7600) { // BIAND #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BIAND #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BIAND #xx:3, @aa:8
            uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry &= (l_operand & (1 << l_imm)) == 0;
}

static void cpuOpcodeBild(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7700) { // BILD #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BILD #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BILD #xx:3, @aa:8
            uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry = (l_operand & (1 << l_imm)) == 0;
}

static void cpuOpcodeBior(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7400) { // BIOR #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BIOR #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BIOR #xx:3, @aa:8
            uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry |= (l_operand & (1 << l_imm)) == 0;
}

static void cpuOpcodeBist(void) {
    int l_imm;
    uint8_t l_operand;
    uint32_t l_address;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6700) { // BIST #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7d00) { // BIST #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            l_address = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_address);
        } else { // BIST #xx:3, @aa:8
            l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    if(s_cpuFlagsRegister.bitField.carry) {
        l_operand |= 1 << l_imm;
    } else {
        l_operand &= 1 << l_imm;
    }

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6700) { // BIST #xx:3.Rd
        cpuSetRegister8(s_cpuOpcodeBuffer[0] & 0x000f, l_operand);
    } else { // BIST #xx:3, @Erd or BIST #xx:3, @aa:8
        busWrite8(l_address, l_operand);
    }
}

static void cpuOpcodeBixor(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7500) { // BIXOR #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BIXOR #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BIXOR #xx:3, @aa:8
            uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry ^= (l_operand & (1 << l_imm)) == 0;
}

static void cpuOpcodeBld(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7700) { // BLD #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BLD #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BLD #xx:3, @aa:8
            uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry = (l_operand & (1 << l_imm)) != 0;
}

static void cpuOpcodeBnot(void) {
    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6100) { // BNOT Rn, Rd
        int l_rn = (s_cpuOpcodeBuffer[0] & 0x00f0) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        int l_mask = 1 << cpuGetRegister8(l_rn);

        cpuSetRegister8(l_rd, cpuGetRegister8(l_rd) ^ l_mask);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7100) { // BNOT #xx:3, Rd
        int l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        int l_mask = 1 << l_imm;

        cpuSetRegister8(l_rd, cpuGetRegister8(l_rd) ^ l_mask);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();

        if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6100) {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7d00) { // BNOT Rn, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
                int l_mask = 1 << cpuGetRegister8(l_rn);

                busWrite8(
                    cpuGetRegister32(l_erd),
                    busRead8(cpuGetRegister32(l_erd)) ^ l_mask
                );
            } else { // BNOT Rn, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
                int l_mask = 1 << cpuGetRegister8(l_rn);

                busWrite8(l_abs, busRead8(l_abs) ^ l_mask);
            }
        } else {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7d00) { // BNOT #xx:3, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
                int l_mask = 1 << l_imm;

                busWrite8(
                    cpuGetRegister32(l_erd),
                    busRead8(cpuGetRegister32(l_erd)) ^ l_mask
                );
            } else { // BNOT #xx:3, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
                int l_mask = 1 << l_imm;

                busWrite8(l_abs, busRead8(l_abs) ^ l_mask);
            }
        }
    }
}

static void cpuOpcodeBor(void) {
    int l_imm;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7400) { // BOR #xx:3, Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BOR #xx:3, @ERd
            int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
            uint32_t l_erdValue = cpuGetRegister32(l_erd);
            l_operand = busRead8(l_erdValue);
        } else { // BOR #xx:3, @aa:8
            uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
            l_operand = busRead8(l_address);
        }
    }

    s_cpuFlagsRegister.bitField.carry |= (l_operand & (1 << l_imm)) != 0;
}

static void cpuOpcodeBset(void) {
    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6000) { // BSET Rn, Rd
        int l_rn = (s_cpuOpcodeBuffer[0] & 0x00f0) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        int l_mask = 1 << cpuGetRegister8(l_rn);

        cpuSetRegister8(l_rd, cpuGetRegister8(l_rd) | l_mask);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7000) { // BSET #xx:3, Rd
        int l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        int l_mask = 1 << l_imm;

        cpuSetRegister8(l_rd, cpuGetRegister8(l_rd) | l_mask);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();

        if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6000) {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7d00) { // BSET Rn, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
                int l_mask = 1 << cpuGetRegister8(l_rn);

                busWrite8(
                    cpuGetRegister32(l_erd),
                    busRead8(cpuGetRegister32(l_erd)) | l_mask
                );
            } else { // BSET Rn, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
                int l_mask = 1 << cpuGetRegister8(l_rn);

                busWrite8(l_abs, busRead8(l_abs) | l_mask);
            }
        } else {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7d00) { // BSET #xx:3, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
                int l_mask = 1 << l_imm;

                busWrite8(
                    cpuGetRegister32(l_erd),
                    busRead8(cpuGetRegister32(l_erd)) | l_mask
                );
            } else { // BSET #xx:3, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
                int l_mask = 1 << l_imm;

                busWrite8(l_abs, busRead8(l_abs) | l_mask);
            }
        }
    }
}

static void cpuOpcodeBsr(void) {
    uint32_t l_disp;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x5500) { // BSR d:8
        l_disp = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
    } else { // BSR d:16
        l_disp = 0xffff0000 | cpuFetch16();
    }

    s_cpuGeneralRegisters[E_CPUREGISTER_ER7].longWord -= 2;

    busWrite16(
        s_cpuGeneralRegisters[E_CPUREGISTER_ER7].longWord,
        s_cpuRegisterPC
    );

    s_cpuRegisterPC += l_disp;
}

static void cpuOpcodeBst(void) {
    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6700) { // BST #xx:3.Rd
        int l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        uint8_t l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
        uint8_t l_mask = (s_cpuFlagsRegister.bitField.carry ? 1 : 0) << l_imm;

        if(s_cpuFlagsRegister.bitField.carry) {
            l_operand |= l_mask;
        } else {
            l_operand &= ~l_mask;
        }

        cpuSetRegister8(s_cpuOpcodeBuffer[0] & 0x000f, l_operand);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
        uint8_t l_mask = 1 << l_imm;
        uint32_t l_address;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7d00) { // BST #xx:3, @ERd
            l_address = cpuGetRegister32((s_cpuOpcodeBuffer[0] & 0x0070) >> 4);
        } else { // BST #xx:3, @aa:8
            l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
        }

        uint8_t l_operand = busRead8(l_address);

        if(s_cpuFlagsRegister.bitField.carry) {
            l_operand |= l_mask;
        } else {
            l_operand &= ~l_mask;
        }

        busWrite8(l_address, l_operand);
    }
}

static void cpuOpcodeBtst(void) {
    uint8_t l_mask;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6300) { // BTST Rn, Rd
        int l_rn = (s_cpuOpcodeBuffer[0] & 0x00f0) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

        l_mask = 1 << cpuGetRegister8(l_rn);
        l_operand = cpuGetRegister8(l_rd);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7300) { // BTST #xx:3, Rd
        int l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        int l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

        l_mask = 1 << l_imm;
        l_operand = cpuGetRegister8(l_rd);
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();

        if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6300) {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7c00) { // BTST Rn, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;

                l_mask = 1 << cpuGetRegister8(l_rn);
                l_operand = busRead8(cpuGetRegister32(l_erd));
            } else { // BTST Rn, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_rn = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;

                l_mask = 1 << cpuGetRegister8(l_rn);
                l_operand = busRead8(l_abs);
            }
        } else {
            if((s_cpuOpcodeBuffer[0] = 0xff00) == 0x7c00) { // BTST #xx:3, @ERd
                int l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

                l_mask = 1 << l_imm;
                l_operand = busRead8(cpuGetRegister32(l_erd));
            } else { // BTST #xx:3, @aa:8
                int l_abs = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
                int l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

                l_mask = 1 << l_imm;
                l_operand = busRead8(l_abs);
            }
        }
    }

    s_cpuFlagsRegister.bitField.zero = (l_operand & l_mask) == 0;
}

static void cpuOpcodeBxor(void) {
    int l_imm;
    uint8_t l_operand;
    uint8_t l_mask;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7500) { // BXOR #xx:3.Rd
        l_imm = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_operand = cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
        l_mask = s_cpuFlagsRegister.bitField.carry << l_imm;
    } else {
        s_cpuOpcodeBuffer[1] = cpuFetch16();
        l_imm = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
        l_mask = 1 << l_imm;
        uint32_t l_address;

        if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7c00) { // BXOR #xx:3, @ERd
            l_address = cpuGetRegister32((s_cpuOpcodeBuffer[0] & 0x0070) >> 4);
        } else { // BXOR #xx:3, @aa:8
            l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
        }

        l_operand = busRead8(l_address);
    }

    s_cpuFlagsRegister.bitField.carry ^= ((l_operand & l_mask) != 0) ? 1 : 0;
}

static void cpuOpcodeCmpB(void) {
    uint8_t l_operand;
    enum te_cpuRegister l_rd;

    if((s_cpuOpcodeBuffer[0] & 0xf000) == 0xa000) { // CMP.B #xx:8, Rd
        l_operand = s_cpuOpcodeBuffer[0];
        l_rd = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 8;
    } else { // CMP.B Rs, Rd
        l_operand = cpuGetRegister8((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    }

    uint8_t l_operand2 = cpuGetRegister8(l_rd);
    uint8_t l_result = l_operand2 - l_operand;

    s_cpuFlagsRegister.bitField.halfCarry =
        (l_operand & 0x0f) > (l_operand2 & 0x0f);
    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        (((l_operand2 ^ l_operand) & ~(l_operand ^ l_result)) & 0x80) != 0;
    s_cpuFlagsRegister.bitField.carry = l_operand > l_operand2;
}

static void cpuOpcodeCmpW(void) {
    uint16_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7900) { // CMP.W #xx:16, Rd
        l_operand = s_cpuOpcodeBuffer[1];
    } else { // CMP.W Rs, Rd
        l_operand = cpuGetRegister16((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);
    }

    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint16_t l_operand2 = cpuGetRegister16(l_rd);
    uint16_t l_result = l_operand2 - l_operand;

    s_cpuFlagsRegister.bitField.halfCarry =
        (l_operand & 0x0fff) > (l_operand2 & 0x0fff);
    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        (((l_operand2 ^ l_operand) & ~(l_operand ^ l_result)) & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.carry = l_operand > l_operand2;
}

static void cpuOpcodeCmpL(void) {
    uint32_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xfff8) == 0x7a20) { // CMP.L #xx:32, ERd
        l_operand = cpuFetch32();
    } else { // CMP.L ERs, ERd
        l_operand = cpuGetRegister32((s_cpuOpcodeBuffer[0] & 0x0070) >> 4);
    }

    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint32_t l_operand2 = cpuGetRegister32(l_erd);
    uint32_t l_result = l_operand2 - l_operand;

    s_cpuFlagsRegister.bitField.halfCarry =
        (l_operand & 0x0fffffff) > (l_operand2 & 0x0fffffff);
    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        (((l_operand2 ^ l_operand) & ~(l_operand ^ l_result)) & 0x80000000)
        != 0;
    s_cpuFlagsRegister.bitField.carry = l_operand > l_operand2;
}

static void cpuOpcodeDaa(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_operand = cpuGetRegister8(l_rd);

    if(s_cpuFlagsRegister.bitField.carry || (l_operand > 0x99)) {
        l_operand += 0x60;
        s_cpuFlagsRegister.bitField.carry = true;
    } else {
        s_cpuFlagsRegister.bitField.carry = false;
    }

    if(s_cpuFlagsRegister.bitField.halfCarry || (l_operand > 0x09)) {
        l_operand += 0x06;
    }

    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80) != 0;

    cpuSetRegister8(l_rd, l_operand);
}

static void cpuOpcodeDas(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_operand = cpuGetRegister8(l_rd);

    if(s_cpuFlagsRegister.bitField.halfCarry || ((l_operand & 0x0f) > 9)) {
        l_operand -= 6;
    }

    if(s_cpuFlagsRegister.bitField.carry || (l_operand > 0x9f)) {
        l_operand -= 0x60;
    }

    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80) != 0;

    cpuSetRegister8(l_rd, l_operand);
}

static void cpuOpcodeDecB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_operand = cpuGetRegister8(l_rd);

    l_operand--;

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = l_operand == 0x7f;

    cpuSetRegister8(l_rd, l_operand);
}

static void cpuOpcodeDecW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint16_t l_operand = cpuGetRegister16(l_rd);
    uint16_t l_operand2;

    if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x1b50) { // DEC.W #1, Rd
        l_operand2 = 1;
    } else { // DEC.W #2, Rd
        l_operand2 = 2;
    }

    uint16_t l_result = l_operand - l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_operand ^ l_result) & 0x8000) != 0;
}

static void cpuOpcodeDecL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    uint32_t l_operand = cpuGetRegister32(l_erd);
    uint32_t l_operand2;

    if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x1b70) { // DEC.L #1, ERd
        l_operand2 = 1;
    } else { // DEC.W #2, Rd
        l_operand2 = 2;
    }

    uint32_t l_result = l_operand - l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_operand ^ l_result) & 0x80000000) != 0;
}

static void cpuOpcodeDivxsB(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[1] & 0x000f;

    int16_t l_dividend = (int16_t)cpuGetRegister16(l_rd);
    int8_t l_divisor = (int8_t)cpuGetRegister8(l_rs);

    int8_t l_quotient = l_dividend / l_divisor;
    int8_t l_remainder = l_dividend % l_divisor;

    s_cpuFlagsRegister.bitField.negative = l_quotient < 0;
    s_cpuFlagsRegister.bitField.zero = l_quotient == 0;

    s_cpuGeneralRegisters[l_rd].byte.rl = l_quotient;
    s_cpuGeneralRegisters[l_rd].byte.rh = l_remainder;
}

static void cpuOpcodeDivxsW(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[1] & 0x000f;

    int32_t l_dividend = (int16_t)cpuGetRegister32(l_rd);
    int16_t l_divisor = (int8_t)cpuGetRegister16(l_rs);

    int16_t l_quotient = l_dividend / l_divisor;
    int16_t l_remainder = l_dividend % l_divisor;

    s_cpuFlagsRegister.bitField.negative = l_quotient < 0;
    s_cpuFlagsRegister.bitField.zero = l_quotient == 0;

    s_cpuGeneralRegisters[l_rd].word.r = l_quotient;
    s_cpuGeneralRegisters[l_rd].word.e = l_remainder;
}

static void cpuOpcodeDivxuB(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[1] & 0x000f;

    uint16_t l_dividend = cpuGetRegister16(l_rd);
    uint8_t l_divisor = cpuGetRegister8(l_rs);

    uint8_t l_quotient = l_dividend / l_divisor;
    uint8_t l_remainder = l_dividend % l_divisor;

    s_cpuFlagsRegister.bitField.negative = (l_quotient & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_quotient == 0;

    s_cpuGeneralRegisters[l_rd].byte.rl = l_quotient;
    s_cpuGeneralRegisters[l_rd].byte.rh = l_remainder;
}

static void cpuOpcodeDivxuW(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[1] & 0x000f;

    uint32_t l_dividend = cpuGetRegister32(l_rd);
    uint16_t l_divisor = cpuGetRegister16(l_rs);

    uint16_t l_quotient = l_dividend / l_divisor;
    uint16_t l_remainder = l_dividend % l_divisor;

    s_cpuFlagsRegister.bitField.negative = (l_quotient & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_quotient == 0;

    s_cpuGeneralRegisters[l_rd].word.r = l_quotient;
    s_cpuGeneralRegisters[l_rd].word.e = l_remainder;
}

static void cpuOpcodeEepmovB(void) {
    uint32_t l_sourceAddress =
        s_cpuGeneralRegisters[E_CPUREGISTER_ER5].longWord;
    uint32_t l_destinationAddress =
        s_cpuGeneralRegisters[E_CPUREGISTER_ER6].longWord;

    while(s_cpuGeneralRegisters[E_CPUREGISTER_R4].byte.rl != 0) {
        busWrite8(l_destinationAddress, busRead8(l_sourceAddress));

        l_destinationAddress++;
        l_sourceAddress++;

        s_cpuGeneralRegisters[E_CPUREGISTER_R4].byte.rl--;
    }

    s_cpuGeneralRegisters[E_CPUREGISTER_ER5].longWord = l_sourceAddress;
    s_cpuGeneralRegisters[E_CPUREGISTER_ER6].longWord = l_destinationAddress;
}

static void cpuOpcodeEepmovW(void) {
    uint32_t l_sourceAddress =
        s_cpuGeneralRegisters[E_CPUREGISTER_ER5].longWord;
    uint32_t l_destinationAddress =
        s_cpuGeneralRegisters[E_CPUREGISTER_ER6].longWord;

    while(s_cpuGeneralRegisters[E_CPUREGISTER_R4].word.r != 0) {
        busWrite8(l_destinationAddress, busRead8(l_sourceAddress));

        l_destinationAddress++;
        l_sourceAddress++;

        s_cpuGeneralRegisters[E_CPUREGISTER_R4].word.r--;
    }

    s_cpuGeneralRegisters[E_CPUREGISTER_ER5].longWord = l_sourceAddress;
    s_cpuGeneralRegisters[E_CPUREGISTER_ER6].longWord = l_destinationAddress;
}

static void cpuOpcodeExtsW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    int16_t l_result = (int8_t)cpuGetRegister16(l_rd);

    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.negative = l_result < 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister16(l_rd, l_result);
}

static void cpuOpcodeExtsL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

    int32_t l_result = (int16_t)cpuGetRegister32(l_erd);

    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.negative = l_result < 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister32(l_erd, l_result);
}

static void cpuOpcodeExtuW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint16_t l_result = (uint8_t)cpuGetRegister16(l_rd);

    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.negative = false;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister16(l_rd, l_result);
}

static void cpuOpcodeExtuL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

    uint32_t l_result = (uint16_t)cpuGetRegister32(l_erd);

    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.negative = false;
    s_cpuFlagsRegister.bitField.overflow = false;

    cpuSetRegister32(l_erd, l_result);
}

static void cpuOpcodeIncB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_operand = cpuGetRegister8(l_rd);

    l_operand--;

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = l_operand == 0x80;

    cpuSetRegister8(l_rd, l_operand);
}

static void cpuOpcodeIncW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint16_t l_operand = cpuGetRegister16(l_rd);
    uint16_t l_operand2;

    if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x0b50) { // INC.W #1, Rd
        l_operand2 = 1;
    } else { // INC.W #2, Rd
        l_operand2 = 2;
    }

    uint16_t l_result = l_operand + l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_operand ^ l_result) & 0x8000) != 0;
}

static void cpuOpcodeIncL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    uint32_t l_operand = cpuGetRegister32(l_erd);
    uint32_t l_operand2;

    if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x0b70) { // INC.L #1, ERd
        l_operand2 = 1;
    } else { // DEC.W #2, Rd
        l_operand2 = 2;
    }

    uint32_t l_result = l_operand + l_operand2;

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_operand ^ l_result) & 0x80000000) != 0;
}

static void cpuOpcodeJmp(void) {
    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x5900) { // JMP @ERn
        s_cpuRegisterPC =
            cpuGetRegister32((s_cpuOpcodeBuffer[0] & 0x0070) >> 4);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x5a00) { // JMP @aa:24
        s_cpuRegisterPC = ((s_cpuOpcodeBuffer[0] & 0x00ff) << 16)
            | cpuFetch16();
    } else { // JMP @@aa:8
        uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
        s_cpuRegisterPC = busRead16(l_address);
    }
}

static void cpuOpcodeJsr(void) {
    s_cpuGeneralRegisters[E_CPUREGISTER_ER7].longWord -= 2;
    busWrite16(
        s_cpuGeneralRegisters[E_CPUREGISTER_ER7].longWord,
        s_cpuRegisterPC
    );

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x5d00) { // JSR @ERn
        s_cpuRegisterPC =
            cpuGetRegister32((s_cpuOpcodeBuffer[0] & 0x0070) >> 4);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x5e00) { // JSR @aa:24
        s_cpuRegisterPC = ((s_cpuOpcodeBuffer[0] & 0x00ff) << 16)
            | cpuFetch16();
    } else { // JSR @@aa:8
        uint32_t l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
        s_cpuRegisterPC = busRead16(l_address);
    }
}

static void cpuOpcodeLdcB(void) {
    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x0700) { // LDC.B #xx:8, CCR
        s_cpuFlagsRegister.byte = s_cpuOpcodeBuffer[0];
    } else { // LDC.B Rs, CCR
        s_cpuFlagsRegister.byte =
            cpuGetRegister8(s_cpuOpcodeBuffer[0] & 0x000f);
    }
}

static void cpuOpcodeLdcW(void) {
    uint32_t l_address;

    s_cpuOpcodeBuffer[1] = cpuFetch16();

    if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6900) { // LDC.W @ERs, CCR
        l_address = cpuGetRegister32((s_cpuOpcodeBuffer[1] & 0x0070) >> 4);
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6f00) { // LDC.W
                                                           // @(d:16, ERs), CCR
        l_address = cpuGetRegister32((s_cpuOpcodeBuffer[1] & 0x0070) >> 4)
            + cpuFetch16();
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7800) { // LDC.W
                                                           // @(d:24, ERs), CCR
        cpuFetch16(); // Discard useless bytes

        uint32_t l_disp = cpuFetch16() << 16;
        l_disp |= cpuFetch16();

        l_address = cpuGetRegister32((s_cpuOpcodeBuffer[1] & 0x0070) >> 4)
            + l_disp;
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6d00) { // LDC.W @ERs+, CCR
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
        l_address = cpuGetRegister32(l_ers);
        cpuSetRegister32(l_ers, l_address + 1);
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6b00) { // LDC.W @aa:16, CCR
        l_address = cpuFetch16();
    } else { // LDC.W @aa:24, CCR
        l_address = cpuFetch16() << 16;
        l_address |= cpuFetch16();
    }

    s_cpuFlagsRegister.byte = busRead16(l_address);
}

static void cpuOpcodeMovB1(void) {
    uint8_t l_value = cpuGetRegister8((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);

    cpuSetRegister8(s_cpuOpcodeBuffer[0] & 0x000f, l_value);

    s_cpuFlagsRegister.bitField.negative = (l_value & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_value == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMovW1(void) {
    uint16_t l_value = cpuGetRegister16((s_cpuOpcodeBuffer[0] & 0x00f0) >> 4);

    cpuSetRegister16(s_cpuOpcodeBuffer[0] & 0x000f, l_value);

    s_cpuFlagsRegister.bitField.negative = (l_value & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_value == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMovL1(void) {
    uint32_t l_value = cpuGetRegister32((s_cpuOpcodeBuffer[0] & 0x0070) >> 4);

    cpuSetRegister32(s_cpuOpcodeBuffer[0] & 0x0007, l_value);

    s_cpuFlagsRegister.bitField.negative = (l_value & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_value == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMovB2(void) {
    uint8_t l_operand;
    enum te_cpuRegister l_rd;

    if((s_cpuOpcodeBuffer[0] & 0xf000) == 0xf000) { // MOV.B #xx:8, Rd
        l_rd = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 4;
        l_operand = s_cpuOpcodeBuffer[0];
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6800) { // MOV.B @ERs, Rd
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

        l_operand = busRead8(cpuGetRegister32(l_ers));
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6e00) { // MOV.B
                                                           // @(d:16, ERs), Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        uint16_t l_disp = cpuFetch16();
        l_operand = busRead8(cpuGetRegister32(l_ers) + l_disp);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7800) { // MOV.B
                                                           // @(d:24, ERs), Rd
        l_rd = s_cpuOpcodeBuffer[1] & 0x000f;
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        uint32_t l_disp = cpuFetch32();

        l_operand = busRead8(cpuGetRegister32(l_ers) + l_disp);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6c00) { // MOV.B @ERs+, Rd
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

        uint32_t l_address = cpuGetRegister32(l_ers);
        l_operand = busRead8(l_address);

        cpuSetRegister32(l_ers, l_address + 1);
    } else if((s_cpuOpcodeBuffer[0] & 0xf000) == 0x2000) { // MOV.B @aa:8, Rd
        l_rd = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 8;
        uint32_t l_address = 0xffffff00 | s_cpuOpcodeBuffer[0];

        l_operand = busRead8(l_address);
    } else if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x6a00) { // MOV.B @aa:16, Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand = busRead8(cpuFetch16());
    } else { // MOV.B @aa:24, Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand = busRead8(cpuFetch32());
    }

    cpuSetRegister8(l_rd, l_operand);

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMovW2(void) {
    uint16_t l_operand;
    enum te_cpuRegister l_rd;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7900) { // MOV.W #xx:16, Rd
        l_operand = cpuFetch16();
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6900) { // MOV.W @ERs, Rd
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

        l_operand = busRead16(cpuGetRegister32(l_ers));
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6f00) { // MOV.W
                                                           // @(d:16, ERs), Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        uint16_t l_disp = cpuFetch16();
        l_operand = busRead16(cpuGetRegister32(l_ers) + l_disp);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7800) { // MOV.W
                                                           // @(d:24, ERs), Rd
        l_rd = s_cpuOpcodeBuffer[1] & 0x000f;
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        uint32_t l_disp = cpuFetch32();

        l_operand = busRead16(cpuGetRegister32(l_ers) + l_disp);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6d00) { // MOV.W @ERs+, Rd
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

        uint32_t l_address = cpuGetRegister32(l_ers);
        l_operand = busRead16(l_address);

        cpuSetRegister32(l_ers, l_address + 2);
    } else if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x6b00) { // MOV.W @aa:16, Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand = busRead16(cpuFetch16());
    } else { // MOV.W @aa:24, Rd
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand = busRead16(cpuFetch32());
    }

    cpuSetRegister16(l_rd, l_operand);

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMovL2(void) {
    uint32_t l_operand;
    enum te_cpuRegister l_erd;

    if(s_cpuOpcodeBuffer[0] == 0x0100) {
        if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6900) { // MOV.L @ERs, ERd
            enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
            l_erd = s_cpuOpcodeBuffer[1] & 0x0007;

            l_operand = busRead32(cpuGetRegister32(l_ers));
        } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6f00) { // MOV.L
                                                               // @(d:16, ERs),
                                                               // ERd
            l_erd = s_cpuOpcodeBuffer[1] & 0x0007;
            enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
            uint16_t l_disp = cpuFetch16();
            l_operand = busRead32(cpuGetRegister32(l_ers) + l_disp);
        } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7800) { // MOV.L
                                                               // @(d:24, ERs),
                                                               // ERd
            l_erd = s_cpuOpcodeBuffer[1] & 0x0007;
            enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
            uint32_t l_disp = cpuFetch32();

            l_operand = busRead32(cpuGetRegister32(l_ers) + l_disp);
        } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6d00) { // MOV.L @ERs+,
                                                               // ERd
            enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
            l_erd = s_cpuOpcodeBuffer[1] & 0x0007;

            uint32_t l_address = cpuGetRegister32(l_ers);
            l_operand = busRead32(l_address);

            cpuSetRegister32(l_ers, l_address + 4);
        } else if((s_cpuOpcodeBuffer[1] & 0xfff0) == 0x6b00) { // MOV.L @aa:16,
                                                               // ERd
            l_erd = s_cpuOpcodeBuffer[1] & 0x0007;
            l_operand = busRead32(cpuFetch16());
        } else { // MOV.L @aa:24, ERd
            l_erd = s_cpuOpcodeBuffer[1] & 0x0007;
            l_operand = busRead32(cpuFetch32());
        }
    } else { // MOV.L #xx:32, ERd
        l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
        l_operand = cpuFetch32();
    }

    cpuSetRegister32(l_erd, l_operand);

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMovB3(void) {
    enum te_cpuRegister l_rs;
    uint32_t l_address;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6800) { // MOV.B Rs, @ERd
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;

        l_address = cpuGetRegister32(l_erd);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6e00) { // MOV.B Rs,
                                                           // @(d:16, ERd)
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        uint16_t l_disp = cpuFetch16();

        l_address = cpuGetRegister32(l_erd) + l_disp;
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7800) { // MOV.B Rs,
                                                           // @(d:24, ERd)
        l_rs = s_cpuOpcodeBuffer[1] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        uint32_t l_disp = cpuFetch32();

        l_address = cpuGetRegister32(l_erd) + l_disp;
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6c00) { // MOV.B Rs, @-ERd
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_address = cpuGetRegister32(l_erd);

        l_address--;

        cpuSetRegister32(l_erd, l_address);
    } else if((s_cpuOpcodeBuffer[0] & 0xf000) == 0x3000) { // MOV.B Rs, @aa:8
        l_rs = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 16;
        l_address = 0xffffff00 | (s_cpuOpcodeBuffer[0] & 0x00ff);
    } else if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x6a80) { // MOV.B Rs, @aa:16
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        l_address = 0xffff0000 | cpuFetch16();
    } else { // MOV.B Rs, @aa:24
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        l_address = cpuFetch32();
    }

    uint8_t l_operand = cpuGetRegister8(l_rs);

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    busWrite8(l_address, l_operand);
}

static void cpuOpcodeMovW3(void) {
    enum te_cpuRegister l_rs;
    uint32_t l_address;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6900) { // MOV.W Rs, @ERd
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;

        l_address = cpuGetRegister32(l_erd);
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6f00) { // MOV.W Rs,
                                                           // @(d:16, ERd)
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_address = cpuGetRegister32(l_erd) + cpuFetch16();
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x7800) { // MOV.W Rs,
                                                           // @(d:24, ERd)
        l_rs = s_cpuOpcodeBuffer[1] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_address = cpuGetRegister32(l_erd) + cpuFetch32();
    } else if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6d00) { // MOV.W Rs, @-ERd
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[0] & 0x0070) >> 4;
        l_address = cpuGetRegister32(l_erd);

        l_address -= 2;

        cpuSetRegister32(l_erd, l_address);
    } else if((s_cpuOpcodeBuffer[0] & 0xfff0) == 0x6b80) { // MOV.W Rs, @aa:16
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        l_address = cpuFetch16();
    } else { // MOV.W Rs, @aa:24
        l_rs = s_cpuOpcodeBuffer[0] & 0x000f;
        l_address = cpuFetch32();
    }

    uint16_t l_operand = cpuGetRegister16(l_rs);

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    busWrite16(l_address, l_operand);
}

static void cpuOpcodeMovL3(void) {
    enum te_cpuRegister l_ers;
    uint32_t l_address;

    if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6900) { // MOV.L ERs, @ERd
        l_ers = s_cpuOpcodeBuffer[1] & 0x0007;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
        l_address = cpuGetRegister32(l_erd);
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6f00) { // MOV.L ERs,
                                                           // @(d:16, ERd)
        l_ers = s_cpuOpcodeBuffer[1] & 0x0007;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        l_address = cpuGetRegister32(l_erd) + cpuFetch16();
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7800) { // MOV.L ERs,
                                                           // @(d:24, ERd)
        l_ers = s_cpuOpcodeBuffer[2] & 0x0007;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;

        l_address = cpuGetRegister32(l_erd) + cpuFetch32();
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6d00) { // MOV.L ERs, @-ERd
        l_ers = s_cpuOpcodeBuffer[2] & 0x0007;
        enum te_cpuRegister l_erd = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
        l_address = cpuGetRegister32(l_erd);

        l_address -= 4;

        cpuSetRegister32(l_erd, l_address);
    } else if((s_cpuOpcodeBuffer[1] & 0xfff0) == 0x6b80) { // MOV.L ERs, @aa:16
        l_ers = s_cpuOpcodeBuffer[2] & 0x0007;
        l_address = cpuFetch16();
    } else { // MOV.L ERs, @aa:24
        l_ers = s_cpuOpcodeBuffer[2] & 0x0007;
        l_address = cpuFetch32();
    }

    uint32_t l_operand = cpuGetRegister32(l_ers);

    s_cpuFlagsRegister.bitField.negative = (l_operand & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_operand == 0;
    s_cpuFlagsRegister.bitField.overflow = false;

    busWrite32(l_address, l_operand);
}

static void cpuOpcodeMovfpe(void) {
    uint16_t l_address = cpuFetch16();
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint8_t l_value = busRead8(l_address);

    cpuSetRegister8(l_rd, l_value);

    s_cpuFlagsRegister.bitField.negative = (l_value & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_value == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMovtpe(void) {
    uint16_t l_address = cpuFetch16();
    enum te_cpuRegister l_rs = s_cpuOpcodeBuffer[0] & 0x000f;

    uint8_t l_value = cpuGetRegister8(l_rs);

    busWrite8(l_address, l_value);

    s_cpuFlagsRegister.bitField.negative = (l_value & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_value == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeMulxsB(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[1] & 0x000f;

    int8_t l_multiplicand = cpuGetRegister16(l_rd);
    int8_t l_multiplier = cpuGetRegister8(l_rs);
    int16_t l_product = l_multiplicand * l_multiplier;

    cpuSetRegister16(l_rd, l_product);

    s_cpuFlagsRegister.bitField.negative = l_product < 0;
    s_cpuFlagsRegister.bitField.zero = l_product == 0;
}

static void cpuOpcodeMulxsW(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[1] & 0x0007;

    int16_t l_multiplicand = cpuGetRegister32(l_erd);
    int16_t l_multiplier = cpuGetRegister16(l_rs);
    int32_t l_product = l_multiplicand * l_multiplier;

    s_cpuFlagsRegister.bitField.negative = l_product < 0;
    s_cpuFlagsRegister.bitField.zero = l_product == 0;

    cpuSetRegister32(l_erd, l_product);
}

static void cpuOpcodeMulxuB(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[1] & 0x000f;

    uint8_t l_multiplicand = cpuGetRegister16(l_rd);
    uint8_t l_multiplier = cpuGetRegister8(l_rs);
    uint16_t l_product = l_multiplicand * l_multiplier;

    cpuSetRegister16(l_rd, l_product);
}

static void cpuOpcodeMulxuW(void) {
    enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[1] & 0x00f0) >> 4;
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[1] & 0x0007;

    uint16_t l_multiplicand = cpuGetRegister32(l_erd);
    uint16_t l_multiplier = cpuGetRegister16(l_rs);
    uint32_t l_product = l_multiplicand * l_multiplier;

    cpuSetRegister32(l_erd, l_product);
}

static void cpuOpcodeNegB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    int8_t l_rdValue = cpuGetRegister8(l_rd);
    int8_t l_result = -l_rdValue;

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.halfCarry = (l_rdValue & 0x0f) != 0;
    s_cpuFlagsRegister.bitField.negative = l_result < 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_rdValue & ~(l_rdValue ^ l_result)) & 0x80) != 0;
    s_cpuFlagsRegister.bitField.carry = l_rdValue > 0;
}

static void cpuOpcodeNegW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    int16_t l_rdValue = cpuGetRegister16(l_rd);
    int16_t l_result = -l_rdValue;

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.halfCarry = (l_rdValue & 0x0fff) != 0;
    s_cpuFlagsRegister.bitField.negative = l_result < 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_rdValue & ~(l_rdValue ^ l_result)) & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.carry = l_rdValue > 0;
}

static void cpuOpcodeNegL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

    int32_t l_rdValue = cpuGetRegister32(l_erd);
    int32_t l_result = -l_rdValue;

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.halfCarry = (l_rdValue & 0x0fffffff) != 0;
    s_cpuFlagsRegister.bitField.negative = l_result < 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_rdValue & ~(l_rdValue ^ l_result)) & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.carry = l_rdValue > 0;
}

static void cpuOpcodeNop(void) {
    // Do nothing
}

static void cpuOpcodeNotB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_result = ~l_rdValue;

    cpuSetRegister8(l_rd, l_rdValue);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeNotW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_result = ~l_rdValue;

    cpuSetRegister8(l_rd, l_rdValue);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeNotL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint8_t l_erdValue = cpuGetRegister32(l_erd);
    uint8_t l_result = ~l_erdValue;

    cpuSetRegister32(l_erd, l_erdValue);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeOrB(void) {
    enum te_cpuRegister l_rd;
    uint8_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x1400) {
        enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[0] & 0x00f0) >> 4;
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand = cpuGetRegister8(l_rs);
    } else {
        l_rd = (s_cpuOpcodeBuffer[0] & 0x0f00) >> 8;
        l_operand = s_cpuOpcodeBuffer[0];
    }

    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_result = l_rdValue | l_operand;

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeOrW(void) {
    enum te_cpuRegister l_rd;
    uint16_t l_operand;

    if((s_cpuOpcodeBuffer[0] & 0xff00) == 0x6400) {
        enum te_cpuRegister l_rs = (s_cpuOpcodeBuffer[0] & 0x00f0) >> 4;
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand = cpuGetRegister16(l_rs);
    } else {
        l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
        l_operand = cpuFetch16();
    }

    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_result = l_rdValue | l_operand;

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeOrL(void) {
    enum te_cpuRegister l_erd;
    uint16_t l_operand;

    if(s_cpuOpcodeBuffer[0] == 0x01f0) {
        enum te_cpuRegister l_ers = (s_cpuOpcodeBuffer[1] & 0x0070) >> 4;
        l_erd = s_cpuOpcodeBuffer[1] & 0x0007;
        l_operand = cpuGetRegister32(l_ers);
    } else {
        l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
        l_operand = cpuFetch32();
    }

    uint32_t l_erdValue = cpuGetRegister32(l_erd);
    uint32_t l_result = l_erdValue | l_operand;

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
}

static void cpuOpcodeOrc(void) {
    uint8_t l_imm = s_cpuOpcodeBuffer[0];

    s_cpuFlagsRegister.byte |= l_imm;
}

static void cpuOpcodeRotlB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_result = (l_rdValue << 1) | (l_rdValue >> 7);

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x80) != 0;
}

static void cpuOpcodeRotlW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_result = (l_rdValue << 1) | (l_rdValue >> 15);

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x8000) != 0;
}

static void cpuOpcodeRotlL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    uint32_t l_erdValue = cpuGetRegister32(l_erd);
    uint32_t l_result = (l_erdValue << 1) | (l_erdValue >> 7);

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x80000000) != 0;
}

static void cpuOpcodeRotrB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_result = (l_rdValue >> 1) | (l_rdValue << 7);

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x01) != 0;
}

static void cpuOpcodeRotrW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_result = (l_rdValue >> 1) | (l_rdValue << 15);

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x0001) != 0;
}

static void cpuOpcodeRotrL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    uint32_t l_erdValue = cpuGetRegister32(l_erd);
    uint32_t l_result = (l_erdValue >> 1) | (l_erdValue << 7);

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x00000001) != 0;
}

static void cpuOpcodeRotxlB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_carry = s_cpuFlagsRegister.bitField.carry ? 1 : 0;
    uint8_t l_result = (l_rdValue << 1) | l_carry;

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x80) != 0;
}

static void cpuOpcodeRotxlW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_carry = s_cpuFlagsRegister.bitField.carry ? 1 : 0;
    uint16_t l_result = (l_rdValue << 1) | l_carry;

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x8000) != 0;
}

static void cpuOpcodeRotxlL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    uint32_t l_erdValue = cpuGetRegister32(l_erd);
    uint32_t l_carry = s_cpuFlagsRegister.bitField.carry ? 1 : 0;
    uint32_t l_result = (l_erdValue << 1) | l_carry;

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x80000000) != 0;
}

static void cpuOpcodeRotxrB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_carry = s_cpuFlagsRegister.bitField.carry ? 1 : 0;
    uint8_t l_result = (l_rdValue >> 1) | (l_carry << 7);

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x01) != 0;
}

static void cpuOpcodeRotxrW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_carry = s_cpuFlagsRegister.bitField.carry ? 1 : 0;
    uint16_t l_result = (l_rdValue >> 1) | (l_carry << 15);

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x0001) != 0;
}

static void cpuOpcodeRotxrL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
    uint32_t l_erdValue = cpuGetRegister32(l_erd);
    uint32_t l_carry = s_cpuFlagsRegister.bitField.carry ? 1 : 0;
    uint32_t l_result = (l_erdValue >> 1) | (l_carry << 31);

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x00000001) != 0;
}

static void cpuOpcodeRte(void) {
    uint32_t l_spValue = cpuGetRegister32(E_CPUREGISTER_ER7);

    s_cpuFlagsRegister.byte = busRead16(l_spValue);
    s_cpuRegisterPC = busRead16(l_spValue + 2);

    cpuSetRegister32(E_CPUREGISTER_ER7, l_spValue + 4);
}

static void cpuOpcodeRts(void) {
    uint32_t l_spValue = cpuGetRegister32(E_CPUREGISTER_ER7);

    s_cpuRegisterPC = busRead16(l_spValue);

    cpuSetRegister32(E_CPUREGISTER_ER7, l_spValue + 2);
}

static void cpuOpcodeShalB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    int8_t l_rdValue = cpuGetRegister8(l_rd);
    int8_t l_result = l_rdValue << 1;

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = ((l_rdValue ^ l_result) & 0x80) != 0;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x80) != 0;
}

static void cpuOpcodeShalW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    int16_t l_rdValue = cpuGetRegister16(l_rd);
    int16_t l_result = l_rdValue << 1;

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_rdValue ^ l_result) & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x8000) != 0;
}

static void cpuOpcodeShalL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

    int32_t l_erdValue = cpuGetRegister32(l_erd);
    int32_t l_result = l_erdValue << 1;

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow =
        ((l_erdValue ^ l_result) & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x80000000) != 0;
}

static void cpuOpcodeSharB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    int8_t l_rdValue = cpuGetRegister8(l_rd);
    int8_t l_result = l_rdValue >> 1;

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x01) != 0;
}

static void cpuOpcodeSharW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    int16_t l_rdValue = cpuGetRegister16(l_rd);
    int16_t l_result = l_rdValue >> 1;

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x0001) != 0;
}

static void cpuOpcodeSharL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

    int32_t l_erdValue = cpuGetRegister32(l_erd);
    int32_t l_result = l_erdValue >> 1;

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x00000001) != 0;
}

static void cpuOpcodeShllB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_result = l_rdValue << 1;

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x80) != 0;
}

static void cpuOpcodeShllW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_result = l_rdValue << 1;

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x8000) != 0;
}

static void cpuOpcodeShllL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

    uint32_t l_erdValue = cpuGetRegister32(l_erd);
    uint32_t l_result = l_erdValue << 1;

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x80000000) != 0;
}

static void cpuOpcodeShlrB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint8_t l_rdValue = cpuGetRegister8(l_rd);
    uint8_t l_result = l_rdValue >> 1;

    cpuSetRegister8(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x01) != 0;
}

static void cpuOpcodeShlrW(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;

    uint16_t l_rdValue = cpuGetRegister16(l_rd);
    uint16_t l_result = l_rdValue >> 1;

    cpuSetRegister16(l_rd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x8000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_rdValue & 0x0001) != 0;
}

static void cpuOpcodeShlrL(void) {
    enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

    uint32_t l_erdValue = cpuGetRegister32(l_erd);
    uint32_t l_result = l_erdValue >> 1;

    cpuSetRegister32(l_erd, l_result);

    s_cpuFlagsRegister.bitField.negative = (l_result & 0x80000000) != 0;
    s_cpuFlagsRegister.bitField.zero = l_result == 0;
    s_cpuFlagsRegister.bitField.overflow = false;
    s_cpuFlagsRegister.bitField.carry = (l_erdValue & 0x00000001) != 0;
}

static void cpuOpcodeStcB(void) {
    enum te_cpuRegister l_rd = s_cpuOpcodeBuffer[0] & 0x000f;
    cpuSetRegister8(l_rd, s_cpuFlagsRegister.byte);
}

static void cpuOpcodeStcW(void) {
    uint32_t l_address;

    if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6900) { // STC.W CCR, @ERd
        enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;

        l_address = cpuGetRegister32(l_erd);
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6f00) { // STC.W CCR,
                                                           // @(d:16, ERd)
        enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
        int16_t l_disp = cpuFetch16();

        l_address = cpuGetRegister32(l_erd) + l_disp;
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x7800) { // STC.W CCR,
                                                           // @(d:24, ERd)
        enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
        cpuFetch16();
        int32_t l_disp = cpuFetch32();

        l_address = cpuGetRegister32(l_erd) + l_disp;
    } else if((s_cpuOpcodeBuffer[1] & 0xff00) == 0x6d00) { // STC.W CCR, @-ERd
        enum te_cpuRegister l_erd = s_cpuOpcodeBuffer[0] & 0x0007;
        l_address = cpuGetRegister32(l_erd);

        l_address -= 2;

        cpuSetRegister32(l_erd, l_address);
    } else if((s_cpuOpcodeBuffer[1] & 0xfff0) == 0x6b80) { // STC.W CCR, @aa:16
        l_address = cpuFetch16();
    } else { // STC.W CCR, @aa:24
        l_address = cpuFetch32();
    }

    busWrite16(l_address, s_cpuFlagsRegister.byte);
}

static void cpuOpcodeSleep(void) {
    // TODO: SLEEP
}

static void cpuOpcodeUndefined(void) {
    // TODO: what happens when an undefined opcode is executed?
}
