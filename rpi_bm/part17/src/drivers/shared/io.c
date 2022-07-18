#include <io.h>

#define MAX_DEVS 10

static io_device *devices[MAX_DEVS] = {0};

bool io_device_register(io_device *dev) {
    for (int i=0; i<MAX_DEVS; i++) {
        if (devices[i] == 0) {
            devices[i] = dev;
            return true;
        }
    }

    return false;
}

io_device *io_device_find(char *name) {
    for (int i=0; i<MAX_DEVS; i++) {
        if (str_eq(devices[i]->name, name)) {
            return devices[i];
        }
    }

    return 0;
}
