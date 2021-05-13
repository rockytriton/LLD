#include <mylib.h>
#include "printf.h"

int main(int argc, char **argv) {
    printf("PrintArgs: %d\n", argc);

    for (int i=1; i<argc; i++) {
        printf("\t%d = %s\n", i, argv[i]);
    }

    sleep_sec(2);

    printf("done\n");

    return 0;
}