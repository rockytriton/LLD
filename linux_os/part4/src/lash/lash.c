#include <mylib.h>
#include <fcntl.h>
#include "printf.h"

unsigned long console_fd = 0;

void console_open() {
    console_fd = sys_open("/dev/console", O_RDWR | O_NDELAY);
}

char console_read() {
    char c = 0;

    while(!sys_read(console_fd, &c, 1)) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10000;
        sys_nanosleep(&ts, NULL);
    }

    return c;
}

void console_write(char c) {
    sys_write(console_fd, &c, 1);
}

int read_line(char *buff, int max) {
    int i=0;

    for (; i<max; i++) {
        char c = console_read();

        if (c == 0) {
            i--;
            continue;
        }

        console_write(c);

        buff[i] = c;

        if (c == '\b') {
            i--;
            buff[i] = 0;
        }

        if (c == '\n') {
            buff[i] = 0;
            return i;
        }
    }

    return i;
}

unsigned long cur_brk = 0;

void process_command(char *cmd) {
    int end = str_pos(cmd, ' ');
    char *arg = 0;

    if (end != -1) {
        cmd[end] = 0;
        arg = cmd + end + 1;
    }

    if (str_eq(cmd, "reboot")) {
        str_print("\n\n*** SYSTEM REBOOTING ***\n");
        sys_reboot();
    }

    if (str_eq(cmd, "brk")) {
        int size = 0;

        if (arg) {
            //brk 4096
            size = str_to_int(arg);
        }

        void *new_val = (void *)(cur_brk + size);
        void *addr = sys_brk(new_val);

        printf("BRK(%X) = %X\n", new_val, addr);

        cur_brk = (unsigned long)sys_brk(0);
    }

    if (str_eq(cmd, "store")) {
        //store ADDRESS VALUE
        //store 1CF0000 12345
        end = str_pos(arg, ' ');
        arg[end] = 0;

        //second arg..
        char *val = arg + end + 1;

        unsigned long addr = hex_str_to_ulong(arg);
        int n = str_to_int(val);

        printf("Storing %d at %X\n", n, addr);

        int *p = (int *)addr;
        *p = n;
    }

    if (str_eq(cmd, "fetch")) {
        //fetch ADDRESS
        unsigned long addr = hex_str_to_ulong(arg);
        int *p = (int *)addr;

        printf("Fetched %d from %X\n", *p, addr);
    }
}


int main() {
    str_print("\033[H\033[J");
    str_print("LASH v0.0.0.2\n");

    cur_brk = (unsigned long)sys_brk(0);
    printf("BRK: %X\n", cur_brk);

    str_print(" :> ");

    console_open();

    while(1) {
        char buff[1024];
        read_line(buff, sizeof(buff));

        process_command(buff);

        str_print(" :> ");
    }
}
