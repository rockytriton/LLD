#include <mylib.h>
#include <fcntl.h>

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

int main() {
    str_print("\033[H\033[J");
    str_print("LASH v0.0.0.1\n");
    str_print(" :> ");

    console_open();

    while(1) {
        char buff[1024];
        read_line(buff, sizeof(buff));

        str_print("INPUT: ");
        str_print(buff);
        str_print("\n");

        if (str_eq(buff, "reboot")) {
            str_print("\n\n*** SYSTEM REBOOTING ***\n");
            sys_reboot();
        }

        str_print(" :> ");
    }
}
