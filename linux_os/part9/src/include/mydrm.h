#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>

struct mydrm_get_cap {
    uint64_t capability;
    uint64_t value;
};

enum mydrm_modes {
    DRM_MODE_CONNECTED = 1,
    DRM_MODE_DISCONNECTED = 2,
    DRM_MODE_UNKNOWN = 3
};

extern int mydrm_ioctl(int fd, unsigned long request, void *arg);
extern int mydrm_open(const char *device_node);
extern int mydrm_get_resources(int fd, struct drm_mode_card_res *res);
extern int mydrm_get_connector(int fd, int id, struct drm_mode_get_connector *conn);

