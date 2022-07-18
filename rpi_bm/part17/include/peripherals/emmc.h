#pragma once

#include "peripherals/base.h"

#include <common.h>
#include <io.h>

#define EMMC_DEBUG 0

#define BSWAP32(x) (((x << 24) & 0xff000000 ) | \
		((x <<  8) & 0x00ff0000 ) | \
		((x >>  8) & 0x0000ff00 ) | \
		((x >> 24) & 0x000000ff ))

typedef struct {
    u8 resp_a : 1;
    u8 block_count : 1;
    u8 auto_command : 2;
    u8 direction : 1;
    u8 multiblock : 1;
    u16 resp_b : 10;
    u8 response_type : 2;
    u8 res0 : 1;
    u8 crc_enable : 1;
    u8 idx_enable : 1;
    u8 is_data : 1;
    u8 type : 2;
    u8 index : 6;
    u8 res1 : 2;
} emmc_cmd;

#define RES_CMD {1, 1, 3, 1, 1, 0xF, 3, 1, 1, 1, 1, 3, 0xF, 3}

typedef enum {
    RTNone,
    RT136,
    RT48,
    RT48Busy
} cmd_resp_type;

typedef enum {
    CTGoIdle = 0,
    CTSendCide = 2,
    CTSendRelativeAddr = 3,
    CTIOSetOpCond = 5,
    CTSelectCard = 7,
    CTSendIfCond = 8,
    CTSetBlockLen = 16,
    CTReadBlock = 17,
    CTReadMultiple = 18,
    CTWriteBlock = 24,
    CTWriteMultiple = 25,
    CTOcrCheck = 41,
    CTSendSCR = 51,
    CTApp = 55
} cmd_type;

typedef struct {
    u32 scr[2];
    u32 bus_widths;
    u32 version;
} scr_register;

typedef enum {
    SDECommandTimeout,
    SDECommandCrc,
    SDECommandEndBit,
    SDECommandIndex,
    SDEDataTimeout,
    SDEDataCrc,
    SDEDataEndBit,
    SDECurrentLimit,
    SDEAutoCmd12,
    SDEADma,
    SDETuning,
    SDERsvd
} sd_error;

typedef struct {
    bool last_success;
    u32 transfer_blocks;
    emmc_cmd last_command;
    reg32 last_command_value;
    u32 block_size;
    u32 last_response[4];
    bool sdhc;
    u16 ocr;
    u32 rca;
    u64 offset;
    void *buffer;
    u32 base_clock;
    u32 last_error;
    u32 last_interrupt;
    scr_register scr;
} emmc_device;

typedef struct  {
    reg32 arg2;
    reg32 block_size_count;
    reg32 arg1;
    reg32 cmd_xfer_mode;
    reg32 response[4];
    reg32 data;
    reg32 status;
    reg32 control[2];
    reg32 int_flags;
    reg32 int_mask;
    reg32 int_enable;
    reg32 control2;
    reg32 cap1;
    reg32 cap2;
    reg32 res0[2];
    reg32 force_int;
    reg32 res1[7];
    reg32 boot_timeout;
    reg32 debug_config;
    reg32 res2[2];
    reg32 ext_fifo_config;
    reg32 ext_fifo_enable;
    reg32 tune_step;
    reg32 tune_SDR;
    reg32 tune_DDR;
    reg32 res3[23];
    reg32 spi_int_support;
    reg32 res4[2];
    reg32 slot_int_status;
} emmc_regs;

#define TO_REG(p) *((reg32 *)p)

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID         400000
#define SD_CLOCK_NORMAL     25000000
#define SD_CLOCK_HIGH       50000000
#define SD_CLOCK_100        100000000
#define SD_CLOCK_208        208000000
#define SD_COMMAND_COMPLETE     1
#define SD_TRANSFER_COMPLETE    (1 << 1)
#define SD_BLOCK_GAP_EVENT      (1 << 2)
#define SD_DMA_INTERRUPT        (1 << 3)
#define SD_BUFFER_WRITE_READY   (1 << 4)
#define SD_BUFFER_READ_READY    (1 << 5)
#define SD_CARD_INSERTION       (1 << 6)
#define SD_CARD_REMOVAL         (1 << 7)
#define SD_CARD_INTERRUPT       (1 << 8)

#if RPI_VERSION == 3
#define EMMC_BASE (PBASE + 0x00300000)
#else
#define EMMC_BASE (PBASE + 0x00340000)
#endif

#define EMMC ((emmc_regs *)EMMC_BASE)

bool emmc_init();
int emmc_read(u8 *buffer, u32 size);
void emmc_seek(u64 offset);

#define EMMC_CTRL1_RESET_DATA (1 << 26)
#define EMMC_CTRL1_RESET_CMD (1 << 25)
#define EMMC_CTRL1_RESET_HOST (1 << 24)
#define EMMC_CTRL1_RESET_ALL (EMMC_CTRL1_RESET_DATA | EMMC_CTRL1_RESET_CMD | EMMC_CTRL1_RESET_HOST)

#define EMMC_CTRL1_CLK_GENSEL (1 << 5)
#define EMMC_CTRL1_CLK_ENABLE (1 << 2)
#define EMMC_CTRL1_CLK_STABLE (1 << 1)
#define EMMC_CTRL1_CLK_INT_EN (1 << 0)

#define EMMC_CTRL0_ALT_BOOT_EN (1 << 22)
#define EMMC_CTRL0_BOOT_EN (1 << 21)
#define EMMC_CTRL0_SPI_MODE (1 << 20)

#define EMMC_STATUS_DAT_INHIBIT (1 << 1)
#define EMMC_STATUS_CMD_INHIBIT (1 << 0)
