#include <mylib.h>
#include <syscall.h>

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

