#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t pid, old_ppid, new_ppid;
    pid_t child, parent;
    parent = getpid();
    if ((child = fork()) < 0) {
        fprintf(stderr, "%s: fork of child failed: %s\n",   argv[0], strerror(errno));
        exit(1);
    }
    else if (child == 0) {
        old_ppid = getppid();
        sleep(2);
        new_ppid = getppid();
    }
    else {
        sleep(1);
        exit(0);
    }
    printf("Original parent: %d\n", parent);
    printf("Child's PID: %d\n", getpid());
    printf("Child's Old PPID: %d\n", old_ppid);
    printf("Child's New PPID: %d\n", new_ppid);
    exit(0);
}
