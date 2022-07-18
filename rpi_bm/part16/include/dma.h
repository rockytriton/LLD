#pragma once

#include <peripherals/dma.h>

typedef struct {
    u32 channel;
    dma_control_block *block;
    bool status;
} dma_channel;

typedef enum {
    CT_NONE = -1,
    CT_NORMAL = 0x81
} dma_channel_type;

dma_channel *dma_open_channel(u32 channel);
void dma_close_channel(dma_channel *channel);
void dma_setup_mem_copy(dma_channel *channel, void *dest, void *src, u32 length, u32 burst_length);
void dma_start(dma_channel *channel);
bool dma_wait(dma_channel *channel);

