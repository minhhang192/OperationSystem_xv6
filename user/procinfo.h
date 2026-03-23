#include "kernel/types.h"

/// @brief Struct save process infomation 
struct procinfo{
    int pid; // Process ID
    int ppid; // Parent Process ID
    int state; // Process state (e.g., running, sleeping, etc.)
    uint64 sz; // Size of process memory (in bytes)
    char name[16]; //Process name (up to 16 characters)
};