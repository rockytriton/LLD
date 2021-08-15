#include "mailbox.h"
#include "printf.h"
#include "timer.h"
#include "video.h"

typedef struct {
    mailbox_tag tag;
    u32 xres;
    u32 yres;
} mailbox_fb_size;

typedef struct {
    mailbox_tag tag;
    u32 bpp;
} mailbox_fb_depth;

typedef struct {
    mailbox_tag tag;
    u32 pitch;
} mailbox_fb_pitch;

typedef struct {
    mailbox_tag tag;
    u32 base; 
    u32 screen_size;
} mailbox_fb_buffer;

typedef struct {
    mailbox_fb_size res;
    mailbox_fb_size vres; //virtual resolution..
    mailbox_fb_depth depth;
    mailbox_fb_buffer buff;
    mailbox_fb_pitch pitch;
} mailbox_fb_request;

static mailbox_fb_request fb_req;

void video_set_resolution(u32 xres, u32 yres, u32 bpp) {
    mailbox_fb_request req;

    fb_req.res.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT;
    fb_req.res.tag.buffer_size = 8;
    fb_req.res.tag.value_length = 8;
    fb_req.res.xres = xres;
    fb_req.res.yres = yres;
    
    fb_req.vres.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT;
    fb_req.vres.tag.buffer_size = 8;
    fb_req.vres.tag.value_length = 8;
    fb_req.vres.xres = xres;
    fb_req.vres.yres = yres;

    fb_req.depth.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_DEPTH;
    fb_req.depth.tag.buffer_size = 4;
    fb_req.depth.tag.value_length = 4;
    fb_req.depth.bpp = bpp;

    fb_req.buff.tag.id = RPI_FIRMWARE_FRAMEBUFFER_ALLOCATE;
    fb_req.buff.tag.buffer_size = 8;
    fb_req.buff.tag.value_length = 4;
    fb_req.buff.base = 16;
    fb_req.buff.screen_size = 0;

    fb_req.pitch.tag.id = RPI_FIRMWARE_FRAMEBUFFER_GET_PITCH;
    fb_req.pitch.tag.buffer_size = 4;
    fb_req.pitch.tag.value_length = 4;
    fb_req.pitch.pitch = 0;

    //sets the actual resolution
    mailbox_process((mailbox_tag *)&fb_req, sizeof(fb_req));

    printf("Allocated Buffer: %X - %d\n", fb_req.buff.base, fb_req.buff.screen_size);

    timer_sleep(2000);
    
    //draw some text showing what resolution is...

    char res[64];
    sprintf(res, "Resolution: %d x %d", xres, yres);
    video_draw_string(res, 20, 20); //upper left corner.

    //draw a blue box with a red box inside it on the screen...

    u32 square_margin = 100;  //blue margin around red center square...

    u64 ms_start = timer_get_ticks() / 1000; //get ticks in ms before drawing on screen...

    for (int y=0; y<yres; y++) {
        if (y == 40) {
            //redraw the "Resolution" string since it's been overwritten by the squares...
            video_draw_string(res, 20, 20);
        }

        for (int x=0; x<xres; x++) {
            //bool for should be draw the red square instead of the blue margin
            bool draw_square = (y > square_margin && yres - y > square_margin) &&
                (x > square_margin && xres - x > square_margin);
            
            video_draw_pixel(x, y, draw_square ? 0xAA0000FF : 0x0055BBFF);
        }
    }

    u64 ms_end = timer_get_ticks() / 1000;
    //ms ticks when done...

    sprintf(res, "Screen draw took %d ms (%d secs)", (ms_end - ms_start), (ms_end - ms_start) / 1000);
    video_draw_string(res, 20, 40);

    printf("DONE\n");

    timer_sleep(2000);
}

void video_draw_pixel(u32 x, u32 y, u32 color) {
    volatile u8 *frame_buffer = (u8 *)((fb_req.buff.base | 0x40000000) & ~0xC0000000);

    u32 pixel_offset = (x * (32 >> 3)) + (y * fb_req.pitch.pitch);

    if (fb_req.depth.bpp == 32) {
        u8 r = (color & 0xFF000000) >> 24;
        u8 g = (color & 0xFF0000) >> 16;
        u8 b = (color & 0xFF00) >> 8;
        u8 a = color & 0xFF;

        frame_buffer[pixel_offset++] = b;
        frame_buffer[pixel_offset++] = g;
        frame_buffer[pixel_offset++] = r;
        frame_buffer[pixel_offset++] = a;
    } else {
        frame_buffer[pixel_offset++] = (color >> 8) & 0xFF;
        frame_buffer[pixel_offset++] = (color & 0xFF);
    }
}

#define TEXT_COLOR 0xFFFFFFFF
#define BACK_COLOR 0x0055BBFF

void video_draw_char(char c, u32 pos_x, u32 pos_y) {
    for (int y=0; y<font_get_height(); y++) {
        for (int x=0; x<font_get_width(); x++) {
            bool yes = font_get_pixel(c, x, y); //gets whether there is a pixel for the font at this pos...
            video_draw_pixel(pos_x + x, pos_y + y, yes ? TEXT_COLOR : BACK_COLOR);
        }
    }
}

void video_draw_string(char *s, u32 pos_x, u32 pos_y) {
    for (int i=0; s[i] != 0; pos_x += (font_get_width() + 2), i++) {
        video_draw_char(s[i], pos_x, pos_y);
    }
}
