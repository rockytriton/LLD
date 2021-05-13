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

        struct event_file *e = malloc(sizeof(struct event_file));
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

char **cmd_to_args(char *cmd) {
    int num_spaces = 0;
    int len = str_len(cmd);

    for (int i=0; i<len; i++) {
        if (cmd[i] == ' ') {
            cmd[i] = 0;
            num_spaces++;
        }
    }

    char **argv = malloc(sizeof(char *) * (num_spaces + 1));
    int cur_item = 0;
    argv[0] = cmd;

    for (int i=0; i<len; i++) {
        if (cmd[i] == 0) {
            argv[++cur_item] = cmd + i + 1;
        }
    }

    argv[++cur_item] = 0;

    return argv;
}

bool find_command(char *fullPath) {
    struct stat statbuf;
    char binPath[64];

    if (sys_stat(fullPath, &statbuf) == 0) {
        return true;
    }

    sprintf(binPath, "/bin/%s", fullPath);
    str_copy(fullPath, binPath);

    if (sys_stat(fullPath, &statbuf) == 0) {
        return true;
    }

    return false;
}

bool process_command(char *cmd) {
    char **argv = cmd_to_args(cmd);

    if (str_eq(cmd, "reboot")) {
        str_print("\n\n*** SYSTEM REBOOTING ***\n");
        sys_reboot();
    } else if (str_eq(cmd, "alloc")) {
        int size = 0;

        if (argv[1]) {
            //brk 4096
            size = str_to_int(argv[1]);
        }

        void *p = malloc(size);

        printf("Returned pointer: %lX\n", p);

        print_heap();
    } else if (str_eq(cmd, "free")) {
        //free ADDRESS
        unsigned long addr = hex_str_to_ulong(argv[1]);
        free(addr);

        print_heap();
    } else if (str_eq(cmd, "store")) {
        //store ADDRESS VALUE
        //store 1CF0000 12345
        //second arg..
        char *val = argv[2];

        unsigned long addr = hex_str_to_ulong(argv[1]);
        int n = str_to_int(val);

        printf("Storing %d at %X\n", n, addr);

        int *p = (int *)addr;
        *p = n;
    } else if (str_eq(cmd, "fetch")) {
        //fetch ADDRESS
        unsigned long addr = hex_str_to_ulong(argv[1]);
        int *p = (int *)addr;

        printf("Fetched %d from %X\n", *p, addr);
    } else if (str_eq(cmd, "events")) {
        handle_events();
    } else {
        struct stat statbuf;
        char fullPath[64];
        str_copy(fullPath, cmd);

        if (find_command(fullPath)) {
            long pid = sys_fork();

            if (pid == 0) {
                char *envp[1];
                envp[0] = 0;

                int ret = sys_execve(fullPath, argv, envp);
                printf("Ret: %d\n", ret);
                
                free(argv);
                _exit(ret);
                
                return false;
            } else {
                siginfo_t info;
                int status = 0;

                int ret = sys_waitid(P_PID, pid, &info, WEXITED);
                printf("Process Returned: %d\n", ret);
            }
        }
    }

    free(argv);
    return true;
}

int main() {
    str_print("\033[H\033[J");
    str_print("LASH v0.0.0.3\n");

    print_heap();

    console_open();
    load_event_devices();

    print_heap();

    str_print(" :> ");

    while(1) {
        char buff[1024];
        read_line(buff, sizeof(buff));

        if (str_eq("", buff)) {
            str_print(" :> ");
            continue;
        }

        if (!process_command(buff)) {
            return 0;
        }

        str_print(" :> ");
    }
}
