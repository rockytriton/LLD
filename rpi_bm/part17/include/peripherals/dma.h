#pragma once

#include "common.h"

#include "peripherals/base.h"

typedef struct {
    u32 transfer_info;
    u32 src_addr;
    u32 dest_addr;
    u32 transfer_length;
    u32 mode_2d_stride;
    u32 next_block_addr;
    u32 res[2];
} dma_control_block;

typedef struct {
    u32 control;
    u32 control_block_addr;
    dma_control_block block;
} dma_channel_regs;

#define REGS_DMA(channel) ((dma_channel_regs *)(PBASE + 0x00007000 + (channel * 0x100)))

#define REGS_DMA_INT_STATUS *((volatile u32 *)(PBASE + 0x00007FE0))
#define REGS_DMA_ENABLE *((volatile u32 *)(PBASE + 0x00007FF0))

//defines for differnet bits of the control and transfer info

#define CS_RESET			(1 << 31)
#define CS_ABORT			(1 << 30)
#define CS_WAIT_FOR_OUTSTANDING_WRITES	(1 << 28)
#define CS_PANIC_PRIORITY_SHIFT		20
    #define DEFAULT_PANIC_PRIORITY		15
#define CS_PRIORITY_SHIFT		16
    #define DEFAULT_PRIORITY		1
#define CS_ERROR			(1 << 8)
#define CS_INT				(1 << 2)
#define CS_END				(1 << 1)
#define CS_ACTIVE			(1 << 0)


#define TI_PERMAP_SHIFT			16
#define TI_BURST_LENGTH_SHIFT		12
#define DEFAULT_BURST_LENGTH		0
#define TI_SRC_IGNORE			(1 << 11)
#define TI_SRC_DREQ			(1 << 10)
#define TI_SRC_WIDTH			(1 << 9)
#define TI_SRC_INC			(1 << 8)
#define TI_DEST_DREQ			(1 << 6)
#define TI_DEST_WIDTH			(1 << 5)
#define TI_DEST_INC			(1 << 4)
#define TI_WAIT_RESP			(1 << 3)
#define TI_TDMODE			(1 << 1)
#define TI_INTEN			(1 << 0)
