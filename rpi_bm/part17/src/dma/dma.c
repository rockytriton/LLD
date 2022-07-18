#include <dma.h>
#include <mem.h>
#include <mm.h>
#include <timer.h>
#include <printf.h>

dma_channel channels[15];

static u16 channel_map = 0x1F35;

static u16 allocate_channel(u32 channel) {
    if (!(channel & ~0x0F)) {
        if (channel_map & (1 << channel)) {
            channel_map &= ~(1 << channel);
            return channel;
        }

        return -1;
    }

    u16 i = channel == CT_NORMAL ? 6 : 12;

    for (; i >= 0; i--) {
        if (channel_map & (1 << i)) {
            channel_map &= ~(1 << i);
            return i;
        }
    }

    return CT_NONE;
}

dma_channel *dma_open_channel(u32 channel) {
    u32 _channel = allocate_channel(channel);

    if (_channel == CT_NONE) {
        printf("INVALID CHANNEL! %d\n", channel);
        return 0;
    }

    dma_channel *dma = (dma_channel *)&channels[_channel];
    dma->channel = _channel;

    dma->block = (dma_control_block *)get_free_pages(1);
    dma->block->res[0] = 0;
    dma->block->res[1] = 0;

    REGS_DMA_ENABLE |= (1 << dma->channel);
    timer_sleep(3);
    REGS_DMA(dma->channel)->control |= CS_RESET;

    while(REGS_DMA(dma->channel)->control & CS_RESET) ;

    return dma;
}

void dma_close_channel(dma_channel *channel) {
    channel_map |= (1 << channel->channel);
}

void dma_setup_mem_copy(dma_channel *channel, void *dest, void *src, u32 length, u32 burst_length) {
    channel->block->transfer_info = (burst_length << TI_BURST_LENGTH_SHIFT)
						    | TI_SRC_WIDTH
						    | TI_SRC_INC
						    | TI_DEST_WIDTH
						    | TI_DEST_INC;

    channel->block->src_addr = (u32)src;
    channel->block->dest_addr = (u32)dest;
    channel->block->transfer_length = length;
    channel->block->mode_2d_stride = 0;
    channel->block->next_block_addr = 0;
}

void dma_start(dma_channel *channel) {
    REGS_DMA(channel->channel)->control_block_addr = BUS_ADDRESS((u32)channel->block);

    REGS_DMA(channel->channel)->control = CS_WAIT_FOR_OUTSTANDING_WRITES
					      | (DEFAULT_PANIC_PRIORITY << CS_PANIC_PRIORITY_SHIFT)
					      | (DEFAULT_PRIORITY << CS_PRIORITY_SHIFT)
					      | CS_ACTIVE;
}

bool dma_wait(dma_channel *channel) {
    while(REGS_DMA(channel->channel)->control & CS_ACTIVE) ;

    channel->status = REGS_DMA(channel->channel)->control & CS_ERROR ? false : true;

    return channel->status;

}
