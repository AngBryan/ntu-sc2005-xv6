#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // We need two pipes:
    int p2c[2]; // Parent-to-Child pipe
    int c2p[2]; // Child-to-Parent pipe
    char buf[5]; // A 5-byte buffer to hold "ping or pong"

    // 1. Create the pipes
    if (pipe(p2c) < 0 || pipe(c2p) < 0) {
        fprintf(2, "pingpong: pipe failed\n");
        exit(1);
    }

    // 2. Fork the process
    int pid = fork();

    if (pid < 0) {
        fprintf(2, "pingpong: fork failed\n");
        exit(1);
    } 
    else if (pid == 0) {
        // -----------------------------------------
        // CHILD PROCESS LOGIC GOES HERE
        // -----------------------------------------
        close(p2c[1]); //close the write for p2c
        close(c2p[0]); //close the read for c2p
        read(p2c[0], buf, 4); //read from p2c file descriptor into buffer buf, read 4 byte.
        printf("%d: received %s\n", getpid(), buf);
        write(c2p[1], "pong", 4);
        close(p2c[0]);
        close(c2p[1]);
        exit(0);

    } 
    else {
        // -----------------------------------------
        // PARENT PROCESS LOGIC GOES HERE
        // -----------------------------------------
        close(p2c[0]); //close the read for p2c
        close(c2p[1]); //close the write for c2p
        write(p2c[1], "ping", 4); //or is it write ping
        read(c2p[0], buf, 4);
        printf("%d: received %s\n", getpid(), buf);
        close(p2c[1]);
        close(c2p[0]);
        wait(0); //parent needs to wait for child process
        exit(0);

    }

    exit(0);
}
