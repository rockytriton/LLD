#include "peripherals/emmc.h"
#include <mailbox.h>
#include <printf.h>

bool wait_reg_mask(reg32 *reg, u32 mask, bool set, u32 timeout);

u32 get_clock_divider(u32 base_clock, u32 target_rate) {
    u32 target_div = 1;

    if (target_rate <= base_clock) {
        target_div = base_clock / target_rate;

        if (base_clock % target_rate) {
            target_div = 0;
        }
    }

    int div = -1;
    for (int fb = 31; fb >= 0; fb--) {
        u32 bt = (1 << fb);

        if (target_div & bt) {
            div = fb;
            target_div &= ~(bt);

            if (target_div) {
                div++;
            }

            break;
        }
    }

    if (div == -1) {
        div = 31;
    }

    if (div >= 32) {
        div = 31;
    }

    if (div != 0) {
        div = (1 << (div - 1));
    }

    if (div >= 0x400) {
        div = 0x3FF;
    }

    u32 freqSel = div & 0xff;
    u32 upper = (div >> 8) & 0x3;
    u32 ret = (freqSel << 8) | (upper << 6) | (0 << 5);

    return ret;
}

bool switch_clock_rate(u32 base_clock, u32 target_rate) {
    u32 divider = get_clock_divider(base_clock, target_rate);

    while((EMMC->status & (EMMC_STATUS_CMD_INHIBIT | EMMC_STATUS_DAT_INHIBIT))) {
        timer_sleep(1);
    }

    u32 c1 = EMMC->control[1] & ~EMMC_CTRL1_CLK_ENABLE;

    EMMC->control[1] = c1;

    timer_sleep(3);

    EMMC->control[1] = (c1 | divider) & ~0xFFE0;

    timer_sleep(3);

    EMMC->control[1] = c1 | EMMC_CTRL1_CLK_ENABLE;

    timer_sleep(3);

    return true;
}

bool emmc_setup_clock() {
    EMMC->control2 = 0;

    u32 rate = mailbox_clock_rate(CT_EMMC);

    u32 n = EMMC->control[1];
    n |= EMMC_CTRL1_CLK_INT_EN;
    n |= get_clock_divider(rate, SD_CLOCK_ID);
    n &= ~(0xf << 16);
    n |= (11 << 16);

    EMMC->control[1] = n;

    if (!wait_reg_mask(&EMMC->control[1], EMMC_CTRL1_CLK_STABLE, true, 2000)) {
        printf("EMMC_ERR: SD CLOCK NOT STABLE\n");
        return false;
    }

    timer_sleep(30);

    //enabling the clock
    EMMC->control[1] |= 4;

    timer_sleep(30);

    return true;
}

