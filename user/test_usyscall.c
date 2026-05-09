#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "user.h"



int main(int argc, char *argv[]){
    int pid = getpid();
    struct usyscall *info = (struct usyscall*) USYSCALL;

    if(ugetpid() == pid) { // Gọi hệ thống để lấy thông tin process
        printf("Process ID from usyscall: %d\n", info->pid);
    } else {
        printf("Failed to get process info for PID %d\n", pid);
    }

    return 0;
}