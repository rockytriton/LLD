#include <mylib.h>
#include <fcntl.h>
#include "printf.h"
#include <linux/input-event-codes.h>

unsigned long console_fd = 0;

struct event_file {
    struct event_file *next;

    int fd;
    char name[64];
};

struct input_event {
        struct timeval time;
        unsigned short type;
        unsigned short code;
        unsigned int value;
};

struct mouse_pos_info {
    int x;
    int y;
    int max_x;
    int max_y;
};

struct mouse_pos_info mouse_pos;

struct event_file *event_list_head;

void load_event_devices() {
    printf("Loading Input Devices...\n");
    event_list_head = NULL;

    mouse_pos.x = 0;
    mouse_pos.y = 0;
    mouse_pos.max_x = 1024;
    mouse_pos.max_y = 768;

    for (int i=0; i<10; i++) {
        char name[64];
        sprintf(name, "/dev/input/event%d", i);

        int fd = sys_open(name, O_RDONLY);

        if (fd < 0) {
            //not found
            break;
        }

        struct event_file *e = mem_alloc(sizeof(struct event_file));
        str_copy(e->name, name);
        e->fd = fd;
        e->next = event_list_head;
        event_list_head = e;
    }
}

bool handle_event(struct event_file *e, struct input_event *event) {

    if (event->type == EV_REL) {
        //mouse relative event.

        if (event->code == REL_X) {
            //mouse X event
            int new_x = mouse_pos.x + event->value;

            if (new_x >= 0 && new_x <= mouse_pos.max_x) {
                mouse_pos.x = new_x;
            }
        }

        if (event->code == REL_Y) {
            //mouse y event
            int new_y = mouse_pos.y + event->value;

            if (new_y >= 0 && new_y <= mouse_pos.max_y) {
                mouse_pos.y = new_y;
            }
        }

        printf("\rMOUSE_POS: %d - %d             ", mouse_pos.x, mouse_pos.y);
    }

    if (event->type == EV_KEY) {
        printf("KEY_INPUT: %s - %d - %d - %d\n", e->name, event->code, event->type, event->value);

        if (event->code == KEY_END) {
            printf("Exiting.\n");
            return true;
        }
    }

    return false;
}

void handle_events() {
    printf("Listening for events...\n");

    while(true) {
        fd_set fds;
        FD_ZERO(&fds);

        struct event_file *e = event_list_head;

        while(e) {
            FD_SET(e->fd, &fds);
            e = e->next;
        }

        int ret = sys_select(event_list_head->fd + 1, &fds, NULL, NULL, NULL);

        if (ret < 0) {
            printf("SELECT FAILED!\n");
            return;
        }

        e = event_list_head;

        while(e) {
            if (FD_ISSET(e->fd, &fds)) {
                char buffer[1024];
                int r = sys_read(e->fd, buffer, sizeof(buffer));
                int pos = 0;

                while(pos < r) {
                    struct input_event *event = (struct input_event *)(buffer + pos);
                    pos += sizeof(struct input_event);

                    if (handle_event(e, event)) {
                        return;
                    }
                }
            }

            e = e->next;
        }
    }
}

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

    if (str_eq(cmd, "events")) {
        handle_events();
    }
}


int main() {
    str_print("\033[H\033[J");
    str_print("LASH v0.0.0.3\n");

    cur_brk = (unsigned long)sys_brk(0);
    printf("BRK: %X\n", cur_brk);

    console_open();
    load_event_devices();

    str_print(" :> ");

    while(1) {
        char buff[1024];
        read_line(buff, sizeof(buff));

        process_command(buff);

        str_print(" :> ");
    }
}
