#include <mydrm.h>
#include <mylib.h>
#include <printf.h>

int main(int argc, char **argv) {
    printf("DRM modes:\n");

    int fd = mydrm_open("/dev/dri/card0");
    struct drm_mode_card_res res;

    if (mydrm_get_resources(fd, &res)) {
        printf("Failed to open card0 resources\n");
        return -1;
    }

    printf("DRM Connectors: %d\n", res.count_connectors);
    sleep_sec(1);

    for (int i=0; i<res.count_connectors; i++) {
        uint32_t *connectors = (uint32_t *)res.connector_id_ptr;

        struct drm_mode_get_connector conn;
        int ret = mydrm_get_connector(fd, connectors[i], &conn);

        if (ret) {
            printf("\tFailed to get connector: %d\n", ret);
            continue;
        }

        printf("Found Connector: %d - %d.  Modes %d\n", i, connectors[i], conn.count_modes);

        if (conn.connection != DRM_MODE_CONNECTED) {
            printf("\tIgnoring unconnected connector. (%d)\n", conn.connection);
            continue;
        }

        struct drm_mode_modeinfo *modes = (struct drm_mode_modeinfo *)conn.modes_ptr;

        for (int m=0; m<conn.count_modes; m++) {
            printf("\tMode: %dx%d\n", modes[m].hdisplay, modes[m].vdisplay);
            sleep_sec(1);
        }
    }

    return 0;
}
