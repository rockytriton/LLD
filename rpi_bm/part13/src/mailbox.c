#include <mailbox.h>
#include <peripherals/base.h>
#include "printf.h"
#include <mem.h>

typedef struct {
    reg32 read;
    reg32 res[5];
    reg32 status;
    reg32 config;
    reg32 write;
} mailbox_regs;

mailbox_regs *MBX() {
    return (mailbox_regs *)(PBASE + 0xB880);
}

typedef struct {
    u32 size;
    u32 code;
    u8 tags[0];
} property_buffer;

static u32 property_data[8192] __attribute__((aligned(16)));

#define MAIL_EMPTY 0x40000000
#define MAIL_FULL  0x80000000

#define MAIL_POWER    0x0 // Mailbox Channel 0: Power Management Interface
#define MAIL_FB       0x1 // Mailbox Channel 1: Frame Buffer
#define MAIL_VUART    0x2 // Mailbox Channel 2: Virtual UART
#define MAIL_VCHIQ    0x3 // Mailbox Channel 3: VCHIQ Interface
#define MAIL_LEDS     0x4 // Mailbox Channel 4: LEDs Interface
#define MAIL_BUTTONS  0x5 // Mailbox Channel 5: Buttons Interface
#define MAIL_TOUCH    0x6 // Mailbox Channel 6: Touchscreen Interface
#define MAIL_COUNT    0x7 // Mailbox Channel 7: Counter
#define MAIL_TAGS     0x8 // Mailbox Channel 8: Tags (ARM to VC)

static void mailbox_write(u8 channel, u32 data) {
    while(MBX()->status & MAIL_FULL) ;

    MBX()->write = (data & 0xFFFFFFF0 | (channel & 0xF));
}

static u32 mailbox_read(u8 channel) {
    while(true) {
        while(MBX()->status & MAIL_EMPTY) ;

        u32 data = MBX()->read;

        u8 read_channel = (u8)(data & 0xF);

        if (read_channel == channel) {
            return data & 0xFFFFFFF0;
        }
    }
}

bool mailbox_process(mailbox_tag *tag, u32 tag_size) {
    int buffer_size = tag_size + 12;

    memcpy(&property_data[2], tag, tag_size);

    property_buffer *buff = (property_buffer *)property_data;
    buff->size = buffer_size;
    buff->code = RPI_FIRMWARE_STATUS_REQUEST;
    property_data[(tag_size + 12) / 4 - 1] = RPI_FIRMWARE_PROPERTY_END;

    mailbox_write(MAIL_TAGS, (u32)(void *)property_data);

    int result = mailbox_read(MAIL_TAGS);

    memcpy(tag, property_data + 2, tag_size);

    return true;
}

bool mailbox_generic_command(u32 tag_id, u32 id, u32 *value) {
    mailbox_generic mbx;
    mbx.tag.id = tag_id;
    mbx.tag.value_length = 0;
    mbx.tag.buffer_size = sizeof(mailbox_generic) - sizeof(mailbox_tag);
    mbx.id = id;
    mbx.value = *value;

    if (!mailbox_process((mailbox_tag *)&mbx, sizeof(mbx))) {
        printf("FAILED TO PROCESS: %X\n", tag_id);
        return false;
    }

    *value = mbx.value;

    return true;
}

u32 mailbox_clock_rate(clock_type ct) {
    mailbox_clock c;
    c.tag.id = RPI_FIRMWARE_GET_CLOCK_RATE;
    c.tag.value_length = 0;
    c.tag.buffer_size = sizeof(c) - sizeof(c.tag);
    c.id = ct;

    mailbox_process((mailbox_tag *)&c, sizeof(c));

    return c.rate;
}

bool mailbox_power_check(u32 type) {
    mailbox_power p;
    p.tag.id = RPI_FIRMWARE_GET_DOMAIN_STATE;
    p.tag.value_length = 0;
    p.tag.buffer_size = sizeof(p) - sizeof(p.tag);
    p.id = type;
    p.state = ~0;

    mailbox_process((mailbox_tag *)&p, sizeof(p));

    return p.state && p.state != ~0;
}
