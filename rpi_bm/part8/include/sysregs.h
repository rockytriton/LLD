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
#define SCR_HCE			(1 << 8)  // hypervisor call enable p.3572
#define SCR_SMD			(1 << 7)  // secure monitor call disable p.3573
#define SCR_RES1_5		(1 << 5)  // reserved as 1 p.3573
#define SCR_RES1_4		(1 << 4)  // reserved as 1 p.3573

#define SCR_RESERVED                    (3 << 4)
#define SCR_RW                          (1 << 10)
#define SCR_NS                          (1 << 0)
#define SCR_VALUE                      (SCR_RW | SCR_HCE | SCR_SMD | SCR_RES1_5 | SCR_RES1_4 | SCR_NS)
// (SCR_RESERVED | SCR_RW | SCR_NS)

//C5.2.19

#define SPSR_MASK_ALL                   (7 << 6)
#define SPSR_EL1h                       (5 << 0)
#define SPSR_EL2h                       (9 << 0)
#define SPSR_VALUE                      (SPSR_MASK_ALL | SPSR_EL1h)

/* Holds the saved process state when an exception is taken to EL3 */
#define SPSR_EL3_D		BIT(9) // debug exception mask
#define SPSR_EL3_A		BIT(8) // SError interrupt mask
#define SPSR_EL3_I		BIT(7) // IRQ interrupt mask
#define SPSR_EL3_F		BIT(6) // FIQ interrupt mask
#define SPSR_EL3_MODE_EL3H	13 // AArch64 Exception level and selected Stack Pointer, 13 is EL.3h.
#define SPSR_EL3_MODE_EL1H  5  // AArch64 Exception level and selected Stack Pointer, 5 is EL.1h.
#define SPSR_EL3_VAL \
    (SPSR_EL3_D | SPSR_EL3_A | SPSR_EL3_I | SPSR_EL3_F | SPSR_EL3_MODE_EL1H)



#define TCR_TG1_4K     (2 << 30)
#define TCR_T1SZ       ((64 - 48) << 16)
#define TCR_TG0_4K     (0 << 14)
#define TCR_T0SZ       (64 - 48)
#define TCR_EL1_VAL    (TCR_TG1_4K | TCR_T1SZ | TCR_TG0_4K | TCR_T0SZ)

/* memory attribute indirect register */
#define MATTR_DEVICE_nGnRnE        0x0
#define MATTR_NORMAL_NC            0x44
#define MATTR_DEVICE_nGnRnE_INDEX  0
#define MATTR_NORMAL_NC_INDEX      1
#define MAIR_EL1_VAL               ((MATTR_NORMAL_NC << (8 * MATTR_NORMAL_NC_INDEX)) | MATTR_DEVICE_nGnRnE << (8 * MATTR_DEVICE_nGnRnE_INDEX))

