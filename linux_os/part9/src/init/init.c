#include <syscall.h>
#include <fcntl.h>
#include <mylib.h>
#include <libfb.h>
#include <linux/vt.h>

uint32_t fix_color(uint32_t num) {
    return (num & 0xFF00FF00) | ((num & 0xFF) << 16) | ((num >> 16) & 0xFF);
}

int main() {
    int fdcon = sys_open("/dev/console", O_RDWR);
    sys_ioctl(fdcon, KDSETMODE, (void *)KD_GRAPHICS);

    struct fb_var_screeninfo vi;
    struct fb_fix_screeninfo fi;

    fb_get_info(&vi, &fi);

    vi.xres = vi.xres_virtual = 1024;
    vi.yres = vi.yres_virtual = 768;

    //add error handling here normally...
    fb_put_info(&vi);

    void *pbuffer = fb_get_buffer();

    int fd = sys_open("/etc/myos.img", O_RDONLY);
    int buff_size = sizeof(int) * 1024 * 768;
    int *buff = malloc(buff_size);
    sys_read(fd, buff, buff_size);
    sys_close(fd);

    int i = 0;

    for (int y=0; y<vi.yres; y++) {
        for (int x=0; x<vi.xres; x++) {
            int location = (x + vi.xoffset) * (vi.bits_per_pixel / 8) + (y + vi.yoffset) * fi.line_length;
            uint32_t *pb = (uint32_t *)(pbuffer + location);
            *pb = fix_color(buff[i]);
            i++;
        }
    }

    sleep_sec(5);
    sys_ioctl(fdcon, KDSETMODE, (void *)KD_TEXT);

    execute_process("/bin/lash");

    while(1) {
        //event loop, for now just tick...
        sleep_sec(1);
    }

    return 0;
}
