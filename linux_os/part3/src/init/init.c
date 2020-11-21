#include <syscall.h>
#include <fcntl.h>
#include <mylib.h>

int main() {
    char *msg = "MyOS 0.0.0.2 Initializing...\n";

    sleep_sec(1);

    str_print(msg);

    sleep_sec(1);

    execute_process("/bin/lash");

    while(1) {
        //event loop, for now just tick...
        sleep_sec(1);
    }

    return 0;
}
