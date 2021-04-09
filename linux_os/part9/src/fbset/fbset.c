#include <mylib.h>
#include <libfb.h>
#include <errno.h>
#include <fcntl.h>
#include "printf.h"

int fill_fb(void *pbuffer, struct fb_var_screeninfo *vi, struct fb_fix_screeninfo *fi, uint32_t color) {
    for (int x=0; x<vi->xres; x++) {
        for (int y=0; y<vi->yres; y++) {
            int location = (x + vi->xoffset) * (vi->bits_per_pixel / 8) + (y + vi->yoffset) * fi->line_length;
            uint32_t *pb = (uint32_t*)(pbuffer + location);
            *pb = color;
        }
    }
}

int main(int argc, char **argv) {
    printf("Frame Buffer Setting...\n");

    if (argc < 3) {
        printf("Usage: fbset <xres> <yres>\n");
        return -1;
    }

    int fdcon = sys_open("/dev/console", O_RDWR);

    struct fb_var_screeninfo vi;
    struct fb_fix_screeninfo fi;

    fb_get_info(&vi, &fi);

    int xres = str_to_int(argv[1]);
    int yres = str_to_int(argv[2]);

    printf("Current resolution: %dx%d\n", vi.xres, vi.yres);
    printf("Setting resolution: %dx%d\n", xres, yres);

    vi.xres = vi.xres_virtual = xres;
    vi.yres = vi.yres_virtual = yres;

    if (fb_put_info(&vi) != FB_SUCCESS) {
        printf("FAILED TO SET BUFFER: %d", errno);
        return -1;
    }

    void *pbuffer = fb_get_buffer();

    if (fb_get_info(&vi, &fi) != FB_SUCCESS) {
        printf("FAILED TO GET VIDEO INFO: %d\n", errno);
        return -1;
    }

    sleep_sec(2);

    fill_fb(pbuffer, &vi, &fi, 0xFFFFFFFF);

    sleep_sec(2);

    sys_ioctl(fdcon, KDSETMODE, (void*)KD_GRAPHICS);

    sleep_sec(2);
    fill_fb(pbuffer, &vi, &fi, 0xFFFF0000);

    sleep_sec(2);
    fill_fb(pbuffer, &vi, &fi, 0xFF00FF00);

    sleep_sec(2);
    fill_fb(pbuffer, &vi, &fi, 0xFF0000FF);

    sleep_sec(2);
    sys_ioctl(fdcon, KDSETMODE, (void*)KD_TEXT);

    return 0;
}
