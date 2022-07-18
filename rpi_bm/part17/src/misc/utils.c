#include "utils.h"
#include "mem.h"

bool str_eq(char *a, char *b) {
    while(*a) {
        if (*a++ != *b++) {
            return false;
        }
    }

    return *a == *b;
}

int strcat(char *dst, char *src) {
    dst += strlen(dst);
    return strcpy(dst, src);
}

int strcpy(char *dst, char *src) {
    int count = 0;

    while(*src) {
        *dst++ = *src++;
        count++;
    }

    *dst = 0;

    return count;
}

int strlen(char *s) {
    int count = 0;
    while(*s++) {
        count++;
    }

    return count;
}
