#include <linux/fb.h>
#include <linux/kd.h>

#define FB_ERR_NOFB0 -1
#define FB_ERR_GETINFO -2
#define FB_ERR_PUTINFO -3
#define FB_ERR_MMAP -4
#define FB_SUCCESS 0

int fb_get_info(struct fb_var_screeninfo *vi, struct fb_fix_screeninfo *fi);
int fb_put_info(struct fb_var_screeninfo *vi);
void *fb_get_buffer();

