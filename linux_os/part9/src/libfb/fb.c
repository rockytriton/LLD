#include <mylib.h>
#include <libfb.h>
#include <fcntl.h>
#include <errno.h>

int fb_get_info(struct fb_var_screeninfo *vi, struct fb_fix_screeninfo *fi) {
    int fd = sys_open("/dev/fb0", O_RDONLY);

    if (fd < 0) {
        errno = fd;
        return FB_ERR_NOFB0;
    }

    int ret;
    if ((ret = sys_ioctl(fd, FBIOGET_VSCREENINFO, vi)) < 0) {
        errno = ret;
        sys_close(fd);
        return FB_ERR_GETINFO;
    }

    if ((ret = sys_ioctl(fd, FBIOGET_FSCREENINFO, fi)) < 0) {
        errno = ret;
        sys_close(fd);
        return FB_ERR_GETINFO;
    }

    sys_close(fd);
    return FB_SUCCESS;
}

int fb_put_info(struct fb_var_screeninfo *vi) {
    int fd = sys_open("/dev/fb0", O_RDWR);

    if (fd < 0) {
        errno = fd;
        return FB_ERR_NOFB0;
    }

    int ret;
    if ((ret = sys_ioctl(fd, FBIOPUT_VSCREENINFO, vi)) < 0) {
        errno = ret;
        sys_close(fd);
        return FB_ERR_PUTINFO;
    }

    sys_close(fd);
    return FB_SUCCESS;
}

void *fb_get_buffer() {
    struct fb_fix_screeninfo fi;
    int fd = sys_open("/dev/fb0", O_RDWR);

    if (fd < 0) {
        errno = fd;
        return (void *)FB_ERR_NOFB0;
    }

    int ret;
    if ((ret = sys_ioctl(fd, FBIOGET_FSCREENINFO, &fi)) < 0) {
        errno = ret;
        sys_close(fd);
        return (void *)FB_ERR_GETINFO;
    }

    void *fb = sys_mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    sys_close(fd);

    return fb;
}
