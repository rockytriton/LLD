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

/*
  .text     .data   .bss    heap    invalid                      stack
 [--------|-------|-------|---------|XXXXXXXXXXXXXXXXXXXXXXXXXXXXX|-----]
                                    ^ brk (0x1FC1000)

  brk(0) = 0x1FC0000
  brk(0x1FC0000 + 0x1000) = 0x1FC1000
*/
void *sys_brk(void *p) {
    return _syscall(SYS_brk, p, 0, 0, 0, 0, 0);
}

int sys_select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout) {
    return _syscall(SYS_select, nfds, readfds, writefds, exceptfds, timeout, 0);
}

void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return _syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}

int sys_munmap(void *addr, size_t length) {
    return _syscall(SYS_munmap, addr, length, 0, 0, 0, 0);
}

int sys_stat(const char *pathname, struct stat *statbuf) {
    return _syscall(SYS_stat, pathname, statbuf, 0, 0, 0, 0);
}

int sys_close(unsigned long fd) {
    return _syscall(SYS_close, fd, 0, 0, 0, 0, 0);
}
