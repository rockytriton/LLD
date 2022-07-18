#include <common.h>
#include <gpio.h>
#include <mailbox.h>
#include <timer.h>
#include <mem.h>
#include <printf.h>
#include <peripherals/emmc.h>
#include <utils.h>
#include <timer.h>

bool emmc_setup_clock();
bool switch_clock_rate(u32 base_clock, u32 target_rate);

static emmc_device device = {0};

static const emmc_cmd INVALID_CMD = RES_CMD;

bool wait_reg_mask(reg32 *reg, u32 mask, bool set, u32 timeout) {
    for (int cycles = 0; cycles <= timeout; cycles++) {
        if ((*reg & mask) ? set : !set) {
            return true;
        }

        timer_sleep(1);
    }

    return false;
}

static const emmc_cmd commands[] = {
    {0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0},
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT136, 0, 1, 0, 0, 0, 2, 0},
    {0, 0, 0, 0, 0, 0, RT48,  0, 1, 0, 0, 0, 3, 0},
    {0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 4, 0},
    {0, 0, 0, 0, 0, 0, RT136, 0, 0, 0, 0, 0, 5, 0},
    {0, 0, 0, 0, 0, 0, RT48,  0, 1, 0, 0, 0, 6, 0},
    {0, 0, 0, 0, 0, 0, RT48Busy,  0, 1, 0, 0, 0, 7, 0},
    {0, 0, 0, 0, 0, 0, RT48,  0, 1, 0, 0, 0, 8, 0},
    {0, 0, 0, 0, 0, 0, RT136, 0, 1, 0, 0, 0, 9, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT48,  0, 1, 0, 0, 0, 16, 0},
    {0, 0, 0, 1, 0, 0, RT48,  0, 1, 0, 1, 0, 17, 0},
    {0, 1, 1, 1, 1, 0, RT48,  0, 1, 0, 1, 0, 18, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT48,  0, 0, 0, 0, 0, 41, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 1, 0, 0, RT48,  0, 1, 0, 1, 0, 51, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT48,  0, 1, 0, 0, 0, 55, 0},
};

static u32 sd_error_mask(sd_error err) {
    return 1 << (16 + (u32)err);
}

static void set_last_error(u32 intr_val) {
    device.last_error = intr_val & 0xFFFF0000;
    device.last_interrupt = intr_val;
}

static bool do_data_transfer(emmc_cmd cmd) {
    u32 wrIrpt = 0;
    bool write = false;

    if (cmd.direction) {
        wrIrpt = 1 << 5;
    } else {
        wrIrpt = 1 << 4;
        write = true;
    }

    u32 *data = (u32 *)device.buffer;

    for (int block = 0; block < device.transfer_blocks; block++) {
        wait_reg_mask(&EMMC->int_flags, wrIrpt | 0x8000, true, 2000);
        u32 intr_val = EMMC->int_flags;
        EMMC->int_flags = wrIrpt | 0x8000;
        
        if ((intr_val & (0xffff0000 | wrIrpt)) != wrIrpt) {
            set_last_error(intr_val);
            return false;
        }


        u32 length = device.block_size;

        if (write) {
            for (; length > 0; length -= 4) {
                EMMC->data = *data++;
            }
        } else {
            for (; length > 0; length -= 4) {
                *data++ = EMMC->data;
            }
        }
    }

    return true;
}

static bool emmc_issue_command(emmc_cmd cmd, u32 arg, u32 timeout) {
    device.last_command_value = TO_REG(&cmd);
    reg32 command_reg = device.last_command_value;

    if (device.transfer_blocks > 0xFFFF) {
        printf("EMMC_ERR: transferBlocks too large: %d\n", device.transfer_blocks);
        return false;
    }

    EMMC->block_size_count = device.block_size | (device.transfer_blocks << 16);
    EMMC->arg1 = arg;
    EMMC->cmd_xfer_mode = command_reg;

    timer_sleep(10);

    int times = 0;

    while(times < timeout) {
        u32 reg = EMMC->int_flags;

        if (reg & 0x8001) {
            break;
        }

        timer_sleep(1);
        times++;
    }

    if (times >= timeout) {
        //just doing a warn for this because sometimes it's ok.
        printf("EMMC_WARN: emmc_issue_command timed out\n");
        device.last_success = false;
        return false;
    }

    u32 intr_val = EMMC->int_flags;

    EMMC->int_flags = 0xFFFF0001;

    if ((intr_val & 0xFFFF0001) != 1) {

        if (EMMC_DEBUG) printf("EMMC_DEBUG: Error waiting for command interrupt complete: %d\n", cmd.index);

        set_last_error(intr_val);

        if (EMMC_DEBUG) printf("EMMC_DEBUG: IRQFLAGS: %X - %X - %X\n", EMMC->int_flags, EMMC->status, intr_val);

        device.last_success = false;
        return false;
    }

    switch(cmd.response_type) {
        case RT48:
        case RT48Busy:
            device.last_response[0] = EMMC->response[0];
            break;

        case RT136:
            device.last_response[0] = EMMC->response[0];
            device.last_response[1] = EMMC->response[1];
            device.last_response[2] = EMMC->response[2];
            device.last_response[3] = EMMC->response[3];
            break;
    }

    if (cmd.is_data) {
        do_data_transfer(cmd);
    }

    if (cmd.response_type == RT48Busy || cmd.is_data) {
        wait_reg_mask(&EMMC->int_flags, 0x8002, true, 2000);
        intr_val = EMMC->int_flags;

        EMMC->int_flags = 0xFFFF0002;

        if ((intr_val & 0xFFFF0002) != 2 && (intr_val & 0xFFFF0002) != 0x100002) {
            set_last_error(intr_val);
            return false;
        }

        EMMC->int_flags = 0xFFFF0002;
    }

    device.last_success = true;

    return true;
}

static bool emmc_command(u32 command, u32 arg, u32 timeout) {
    if (command & 0x80000000) {
        //The app command flag is set, shoudl use emmc_app_command instead.
        printf("EMMC_ERR: COMMAND ERROR NOT APP\n");
        return false;
    }

    device.last_command = commands[command];

    if (TO_REG(&device.last_command) == TO_REG(&INVALID_CMD)) {
        printf("EMMC_ERR: INVALID COMMAND!\n");
        return false;
    }

    return emmc_issue_command( device.last_command, arg, timeout);
}

static bool reset_command() {
    EMMC->control[1] |= EMMC_CTRL1_RESET_CMD;

    for (int i=0; i<10000; i++) {
        if (!( EMMC->control[1] & EMMC_CTRL1_RESET_CMD)) {
            return true;
        }

        timer_sleep(1);
    }

    printf("EMMC_ERR: Command line failed to reset properly: %X\n",  EMMC->control[1]);

    return false;
}

bool emmc_app_command(u32 command, u32 arg, u32 timeout) {

    if (commands[command].index >= 60) {
        printf("EMMC_ERR: INVALID APP COMMAND\n");
        return false;
    }

    device.last_command = commands[CTApp];

    u32 rca = 0;

    if (device.rca) {
        rca = device.rca << 16;
    }

    if (emmc_issue_command( device.last_command, rca, 2000)) {
        device.last_command = commands[command];

        return emmc_issue_command( device.last_command, arg, 2000);
    }

    return false;
}

static bool check_v2_card() {
    bool v2Card = false;

    if (!emmc_command( CTSendIfCond, 0x1AA, 200)) {
        if (device.last_error == 0) {
            //timeout.
            printf("EMMC_ERR: SEND_IF_COND Timeout\n");
        } else if (device.last_error & (1 << 16)) {
            //timeout command error
            if (!reset_command()) {
                return false;
            }

            EMMC->int_flags = sd_error_mask(SDECommandTimeout);
            printf("EMMC_ERR: SEND_IF_COND CMD TIMEOUT\n");
        } else {
            printf("EMMC_ERR: Failure sending SEND_IF_COND\n");
            return false;
        }
    } else {
        if ((device.last_response[0] & 0xFFF) != 0x1AA) {
            printf("EMMC_ERR: Unusable SD Card: %X\n", device.last_response[0]);
            return false;
        }

        v2Card = true;
    }

    return v2Card;
}

static bool check_usable_card() {
    if (!emmc_command( CTIOSetOpCond, 0, 1000)) {
        if (device.last_error == 0) {
            //timeout.
            printf("EMMC_ERR: CTIOSetOpCond Timeout\n");
        } else if (device.last_error & (1 << 16)) {
            //timeout command error
            //this is a normal expected error and calling the reset command will fix it.
            if (!reset_command()) {
                return false;
            }

            EMMC->int_flags = sd_error_mask(SDECommandTimeout);
        } else {
            printf("EMMC_ERR: SDIO Card not supported\n");
            return false;
        }
    }

    return true;
}

static bool check_sdhc_support(bool v2_card) {
    bool card_busy = true;

    while(card_busy) {
        u32 v2_flags = 0;

        if (v2_card) {
            v2_flags |= (1 << 30); //SDHC Support
        }

        if (!emmc_app_command( CTOcrCheck, 0x00FF8000 | v2_flags, 2000)) {
            printf("EMMC_ERR: APP CMD 41 FAILED 2nd\n");
            return false;
        }

        if (device.last_response[0] >> 31 & 1) {
            device.ocr = (device.last_response[0] >> 8 & 0xFFFF);
            device.sdhc = ((device.last_response[0] >> 30) & 1) != 0;
            card_busy = false;
        } else {
            if (EMMC_DEBUG) printf("EMMC_DEBUG: SLEEPING: %X\n", device.last_response[0]);
            timer_sleep(500);
        }
    }

    return true;
}

static bool check_ocr() {
    bool passed = false;

    for (int i=0; i<5; i++) {
        if (!emmc_app_command(CTOcrCheck, 0, 2000)) {
            printf("EMMC_WARN: APP CMD OCR CHECK TRY %d FAILED\n", i + 1);
            passed = false;
        } else {
            passed = true;
        }

        if (passed) {
            break;
        }

        return false;
    }

    if (!passed) {
        printf("EMMC_ERR: APP CMD 41 FAILED\n");
        return false;
    }

    device.ocr = (device.last_response[0] >> 8 & 0xFFFF);

    if (EMMC_DEBUG) printf("MEMORY OCR: %X\n", device.ocr);

    return true;
}

static bool check_rca() {
    if (!emmc_command( CTSendCide, 0, 2000)) {
        printf("EMMC_ERR: Failed to send CID\n");

        return false;
    }

    if (EMMC_DEBUG) printf("EMMC_DEBUG: CARD ID: %X.%X.%X.%X\n", device.last_response[0], device.last_response[1], device.last_response[2], device.last_response[3]);

    if (!emmc_command( CTSendRelativeAddr, 0, 2000)) {
        printf("EMMC_ERR: Failed to send Relative Addr\n");

        return false;
    }

    device.rca = (device.last_response[0] >> 16) & 0xFFFF;
    
    if (EMMC_DEBUG) {
        printf("EMMC_DEBUG: RCA: %X\n", device.rca);

        printf("EMMC_DEBUG: CRC_ERR: %d\n", (device.last_response[0] >> 15) & 1);
        printf("EMMC_DEBUG: CMD_ERR: %d\n", (device.last_response[0] >> 14) & 1);
        printf("EMMC_DEBUG: GEN_ERR: %d\n", (device.last_response[0] >> 13) & 1);
        printf("EMMC_DEBUG: STS_ERR: %d\n", (device.last_response[0] >> 9) & 1);
        printf("EMMC_DEBUG: READY  : %d\n", (device.last_response[0] >> 8) & 1);
    }

    if (!((device.last_response[0] >> 8) & 1)) {
        printf("EMMC_ERR: Failed to read RCA\n");
        return false;
    }

    return true;
}

static bool select_card() {
    if (!emmc_command( CTSelectCard, device.rca << 16, 2000)) {
        printf("EMMC_ERR: Failed to select card\n");
        return false;
    }

    if (EMMC_DEBUG) printf("EMMC_DEBUG: Selected Card\n");

    u32 status = (device.last_response[0] >> 9) & 0xF;

    if (status != 3 && status != 4) {
        printf("EMMC_ERR: Invalid Status: %d\n", status);
        return false;
    }

    if (EMMC_DEBUG) printf("EMMC_DEBUG: Status: %d\n", status);

    return true;
}

static bool set_scr() {
    if (!device.sdhc) {
        if (!emmc_command( CTSetBlockLen, 512, 2000)) {
            printf("EMMC_ERR: Failed to set block len\n");
            return false;
        }
    }

    u32 bsc = EMMC->block_size_count;
    bsc &= ~0xFFF; //mask off bottom bits
    bsc |= 0x200; //set bottom bits to 512
    EMMC->block_size_count = bsc;

    device.buffer = &device.scr.scr[0];
    device.block_size = 8;
    device.transfer_blocks = 1;

    if (!emmc_app_command( CTSendSCR, 0, 30000)) {
        printf("EMMC_ERR: Failed to send SCR\n");
        return false;
    }

    if (EMMC_DEBUG) printf("EMMC_DEBUG: GOT SRC: SCR0: %X SCR1: %X BWID: %X\n", device.scr.scr[0], device.scr.scr[1], device.scr.bus_widths);

    device.block_size = 512;

    u32 scr0 = BSWAP32(device.scr.scr[0]);
    device.scr.version = 0xFFFFFFFF;
    u32 spec = (scr0 >> (56 - 32)) & 0xf;
    u32 spec3 = (scr0 >> (47 - 32)) & 0x1;
    u32 spec4 = (scr0 >> (42 - 32)) & 0x1;

    if (spec == 0) {
        device.scr.version = 1;
    } else if (spec == 1) {
        device.scr.version = 11;
    } else if (spec == 2) {

        if (spec3 == 0) {
            device.scr.version = 2;
        } else if (spec3 == 1) {
            if (spec4 == 0) {
                device.scr.version = 3;
            }
            if (spec4 == 1) {
                device.scr.version = 4;
            }
        }
    }

    if (EMMC_DEBUG) printf("EMMC_DEBUG: SCR Version: %d\n", device.scr.version);

    return true;
}

static bool emmc_card_reset() {
    EMMC->control[1] = EMMC_CTRL1_RESET_HOST;

    if (EMMC_DEBUG) printf("EMMC_DEBUG: Card resetting...\n");

    if (!wait_reg_mask(&EMMC->control[1], EMMC_CTRL1_RESET_ALL, false, 2000)) {
        printf("EMMC_ERR: Card reset timeout!\n");
        return false;
    }

    #if (RPI_VERSION == 4)
    //This enabled VDD1 bus power for SD card, needed for RPI 4.
    u32 c0 = EMMC->control[0];
    c0 |= 0x0F << 8;
    EMMC->control[0] = c0;
    timer_sleep(3);
    #endif

    if (!emmc_setup_clock()) {
        return false;
    }

    //All interrupts go to interrupt register.
    EMMC->int_enable = 0;
    EMMC->int_flags = 0xFFFFFFFF;
    EMMC->int_mask = 0xFFFFFFFF;

    timer_sleep(203);

    device.transfer_blocks = 0;
    device.last_command_value = 0;
    device.last_success = false;
    device.block_size = 0;

    if (!emmc_command( CTGoIdle, 0, 2000)) {
        printf("EMMC_ERR: NO GO_IDLE RESPONSE\n");
        return false;
    }

    bool v2_card = check_v2_card();    

    if (!check_usable_card()) {
        return false;
    }

    if (!check_ocr()) {
        return false;
    }

    if (!check_sdhc_support(v2_card)) {
        return false;
    }

    switch_clock_rate(device.base_clock, SD_CLOCK_NORMAL);

    timer_sleep(10);

    if (!check_rca()) {
        return false;
    }

    if (!select_card()) {
        return false;
    }

    if (!set_scr()) {
        return false;
    }

    //enable all interrupts
    EMMC->int_flags = 0xFFFFFFFF;

    if (EMMC_DEBUG) printf("EMMC_DEBUG: Card reset!\n");

    return true;
}

int emmc_io_read(io_device *dev, void *b, u32 size) {
    return emmc_read((u8 *)b, size);
}

void emmc_io_seek(io_device *dev, u64 offset) {
    return emmc_seek(offset);
}

bool do_data_command(bool write, u8 *b, u32 bsize, u32 block_no) {
    if (!device.sdhc) {
        block_no *= 512;
    }

    if (bsize < device.block_size) {
        printf("EMMC_ERR: INVALID BLOCK SIZE: \n", bsize, device.block_size);
        return false;
    }

    device.transfer_blocks = bsize / device.block_size;

    if (bsize % device.block_size) {
        printf("EMMC_ERR: BAD BLOCK SIZE\n");
        return false;
    }

    device.buffer = b;

    cmd_type command = CTReadBlock;

    if (write && device.transfer_blocks > 1) {
        command = CTWriteMultiple;
    } else if (write) {
        command = CTWriteBlock;
    } else if (!write && device.transfer_blocks > 1) {
        command = CTReadMultiple;
    } 

    int retry_count = 0;
    int max_retries = 3;

    if (EMMC_DEBUG) printf("EMMC_DEBUG: Sending command: %d\n", command);

    while(retry_count < max_retries) {
        if (emmc_command( command, block_no, 5000)) {
            break;
        }

        if (++retry_count < max_retries) {
            printf("EMMC_WARN: Retrying data command\n");
        } else {
            printf("EMMC_ERR: Giving up data command\n");
            return false;
        }
    }

    return true;
}

int do_read(u8 *b, u32 bsize, u32 block_no) {
    //TODO ENSURE DATA MODE...

    if (!do_data_command( false, b, bsize, block_no)) {
        printf("EMMC_ERR: do_data_command failed\n");
        return -1;
    }

    return bsize;
}


int emmc_read(u8 *buffer, u32 size) {
    if (device.offset % 512 != 0) {
        printf("EMMC_ERR: INVALID OFFSET: %d\n", device.offset);
        return -1;
    }

    u32 block = device.offset / 512;

    int r = do_read( buffer, size, block);

    if (r != size) {
        printf("EMMC_ERR: READ FAILED: %d\n", r);
        return -1;
    }

    return size;
}

void emmc_seek(u64 _offset) {
    device.offset = _offset;
}

static io_device emmc_io_device = {
    .name = "disk",
    .data = &device,
    .read = emmc_io_read,
    .seek = emmc_io_seek
};

bool emmc_init() {
    io_device_register(&emmc_io_device);

    gpio_pin_set_func(34, GFInput);
    gpio_pin_set_func(35, GFInput);
    gpio_pin_set_func(36, GFInput);
    gpio_pin_set_func(37, GFInput);
    gpio_pin_set_func(38, GFInput);
    gpio_pin_set_func(39, GFInput);

    gpio_pin_set_func(48, GFAlt3);
    gpio_pin_set_func(49, GFAlt3);
    gpio_pin_set_func(50, GFAlt3);
    gpio_pin_set_func(51, GFAlt3);
    gpio_pin_set_func(52, GFAlt3);
    
    device.transfer_blocks = 0;
    device.last_command_value = 0;
    device.last_success = false;
    device.block_size = 0;
    device.sdhc = false;
    device.ocr = 0;
    device.rca = 0;
    device.offset = 0;
    device.base_clock = 0;

    bool success = false;
    for (int i=0; i<10; i++) {
        success = emmc_card_reset();

        if (success) {
            break;
        }

        timer_sleep(100);
        printf("EMMC_WARN: Failed to reset card, trying again...\n");
    }

    if (!success) {
        return false;
    }

    return true;
}
