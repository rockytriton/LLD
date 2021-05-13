#include "log.h"
#include <stdio.h>

int console_init(logger *self, void *data) {
    printf("Console Logger Init\n");
    return 0;
}

void console_info(logger *self, char *str) {
    printf("%s\n", str);
}

void console_error(logger *self, char *str) {
    fprintf(stderr, "%s\n", str);
}

void console_close(logger *self) {

}

log_interface console_interface = {
    .init = console_init,
    .info = console_info,
    .error = console_error,
    .close = console_close
};

logger conlog = {
    .interface = &console_interface
};
