#include "user.h"
#include "kernel/types.h"
#include "procinfo.h"

int main(int argc, char *argv[]){
    int pid = getpid();
    struct procinfo info;

    if (getproc(pid, &info) == 0) { // Gọi hệ thống để lấy thông tin process
        printf("Process Info:\n");
        printf("PID: %d\n", info.pid);
        printf("PPID: %d\n", info.ppid);
        printf("State: %d\n", info.state);
        printf("Size: %ld bytes\n", info.sz);
        printf("Name: %s\n", info.name);
    } else {
        printf("Failed to get process info for PID %d\n", pid);
    }

    return 0;
}