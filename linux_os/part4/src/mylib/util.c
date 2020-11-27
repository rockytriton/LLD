#include <mylib.h>
#include <syscall.h>
#include "printf.h"

unsigned long str_len(char *sz) {
    int count = 0;

    while(*sz++) {
        count++;
    }

    return count;
}

void delay(int ticks) {
    for (int i=0; i<ticks; i++) {
        //nothing...
    }
}

void str_print(char *str) {
    _syscall(SYS_write, (void *)1 /*stdout*/, str, (void *)str_len(str), 0, 0, 0);
}

int str_eq(char *a, char *b) {
    char ca = *a;
    char cb = *b;

    for (; ca != 0 && cb != 0; ca = *(++a), cb = *(++b)) {
        if (ca != cb) {
            return 0;
        }
    }

    return ca == cb;
}

int str_pos(char *s, char c) {
    for (int i=0; s[i]; i++) {
        if (s[i] == c) {
            return i;
        }
    }

    return -1;
}

unsigned long hex_str_to_ulong(char *s) {
    static char *hexchars = "0123456789ABCDEF";
    unsigned long result = 0;

    for (; *s; s++) {
        result *= 0x10;
        result += str_pos(hexchars, *s);
    }

    return result;
}

int str_to_int(char *s) {
    bool neg = *s == '-';
    int result = 0;

    if (neg) {
        s++;
    }

    for (; *s; s++) {
        result *= 10;
        result += *s - '0';
    }

    return neg ? -result : result;
}
