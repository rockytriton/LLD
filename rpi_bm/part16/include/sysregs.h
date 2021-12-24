#pragma once

//D13.2.113

#define SCTLR_RESERVED                  (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN          (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN         (0 << 24)
#define SCTLR_I_CACHE_DISABLED          (0 << 12)
#define SCTLR_D_CACHE_DISABLED          (0 << 2)
#define SCTLR_MMU_DISABLED              (0 << 0)
#define SCTLR_MMU_ENABLED               (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

//D13.2.47

#define HCR_RW                          (1 << 31)
#define HCR_VALUE                       HCR_RW

//D13.2.112

#define SCR_RESERVED                    (3 << 4)
#define SCR_RW                          (1 << 10)
#define SCR_NS                          (1 << 0)
#define SCR_VALUE                       (SCR_RESERVED | SCR_RW | SCR_NS)

//C5.2.19

#define SPSR_MASK_ALL                   (7 << 6)
#define SPSR_EL1h                       (5 << 0)
#define SPSR_EL2h                       (9 << 0)
#define SPSR_VALUE                      (SPSR_MASK_ALL | SPSR_EL1h)


#define ESR_ELx_EC_SHIFT		26
#define ESR_ELx_EC_SVC64		0x15
#define ESR_ELx_EC_DABT_LOW		0x24

/* Holds the saved process state when an exception is taken to EL3 */
#define SPSR_EL3_D		(1 << 9) // debug exception mask
#define SPSR_EL3_A		(1 << 8) // SError interrupt mask
#define SPSR_EL3_I		(1 << 7) // IRQ interrupt mask
#define SPSR_EL3_F		(1 << 6) // FIQ interrupt mask
#define SPSR_EL3_MODE_EL3H	13 // AArch64 Exception level and selected Stack Pointer, 13 is EL.3h.
#define SPSR_EL3_MODE_EL1H  5  // AArch64 Exception level and selected Stack Pointer, 5 is EL.1h.
#define SPSR_EL3_VAL  (SPSR_EL3_D | SPSR_EL3_A | SPSR_EL3_I | SPSR_EL3_F | SPSR_EL3_MODE_EL1H)

#define TCR_TG1_4K     (2 << 30)
#define TCR_T1SZ       ((64 - 48) << 16)
#define TCR_TG0_4K     (0 << 14)
#define TCR_T0SZ       (64 - 48)
#define TCR_EL1_VAL    (TCR_TG1_4K | TCR_T1SZ | TCR_TG0_4K | TCR_T0SZ)

/* architectural feature access control register */
#define CPACR_EL1_FPEN    (1 << 21) | (1 << 20) // don't trap SIMD/FP registers
#define CPACR_EL1_ZEN     (1 << 17) | (1 << 16)  // don't trap SVE instructions
#define CPACR_EL1_VAL     (CPACR_EL1_FPEN | CPACR_EL1_ZEN)

/* exception syndrome register EL1 (ESR_EL1) */
#define ESR_ELx_EC_SHIFT 26
#define ESR_ELx_EC_SVC64 0x15
#define ESR_ELx_EC_DA_LOW 0x24
