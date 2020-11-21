#include <syscall.h>
#include <mylib.h>

unsigned long sys_open(char *fn, int flags) {
    return _syscall(SYS_open, fn, (void *)(long)flags, 0, 0, 0, 0);
}

unsigned long sys_read(unsigned long fd, char *buff, unsigned long size) {
    return _syscall(SYS_read, (void *)fd, buff, (void *)size, 0, 0, 0);
}

unsigned long sys_reboot() {
    return _syscall(SYS_reboot, (void *)0xfee1dead, (void *)672274793, (void*)0x1234567, 0, 0, 0);
}

unsigned long sys_nanosleep(struct timespec *req, struct timespec *rem) {
    return _syscall(SYS_nanosleep, req, rem, 0, 0, 0, 0);
}

void sleep_sec(int sec) {
    struct timespec tm;
    tm.tv_nsec = 0;
    tm.tv_sec = sec;

    sys_nanosleep(&tm, NULL);
}

unsigned long sys_write(unsigned long fd, char *buf, unsigned long len) {
    return _syscall(SYS_write, fd, buf, len, 0, 0, 0);
}

long sys_fork() {
    return _syscall(SYS_fork, 0, 0, 0, 0, 0, 0);
}

long sys_execve(char *filename, char **argv, char **envp) {
    return _syscall(SYS_execve, filename, argv, envp, 0, 0, 0);
}

int execute_process(char *filename) {
    long pid = sys_fork();

    if (!pid) {
        char *argv[2];
        argv[0] = filename;
        argv[1] = 0;

        char *envp[1];
        envp[0] = 0;

        return sys_execve(filename, argv, envp);
    }
}


