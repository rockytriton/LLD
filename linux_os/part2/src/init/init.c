#include <syscall.h>
#include <fcntl.h>
#include <mylib.h>

int main() {
    char *msg = "MyOS 0.0.0.1 Initializing...\n";

    sleep_sec(1);

    str_print(msg);

    sleep_sec(1);

    char buff[255];
    char *filename = "/src/init.c";

    str_print("Opening file: ");
    str_print(filename);
    str_print("\n");

    unsigned long fd = sys_open(filename, O_RDONLY);

    sys_read(fd, buff, sizeof(buff));

    str_print(buff);

    for (int t=0; t<3; t++) {
        //event loop, for now just tick...
        sleep_sec(1);
        str_print("TICK!\n");
    }

    sys_reboot();

    return 0;
}
