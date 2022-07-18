#pragma once

#include <common.h>

typedef struct _io_device {
    char *name;
    void *data;
    int (*read)(struct _io_device *, void *, u32);
    int (*write)(struct _io_device *, void *, u32);
    void (*seek)(struct _io_device *, u64);
    bool (*open)(struct _io_device *, void *);
    bool (*close)(struct _io_device *, void *);
} io_device;

bool io_device_register(io_device *dev);

io_device *io_device_find(char *name);
