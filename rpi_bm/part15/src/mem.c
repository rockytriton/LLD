#include <mem.h>

void *memcpy(void *dest, const void *src, u32 n) {
    //simple implementation...
    u8 *bdest = (u8 *)dest;
    u8 *bsrc = (u8 *)src;

    for (int i=0; i<n; i++) {
        bdest[i] = bsrc[i];
    }

    return dest;
}
