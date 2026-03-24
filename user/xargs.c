#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    // kiểm tra số lượng tham số
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }

    char *cmd_argv[MAXARG]; // mảng chứa các tham số cho lệnh sẽ thực thi
    int base_argc = 0; // số lượng tham số

    // lấy các tham số từ dòng lệnh và lưu vào cmd_argv
    for (int i = 1; i < argc; i++) {
        cmd_argv[base_argc++] = argv[i];
    }

    char buffer[512]; // mảng để lưu trữ dòng nhập từ stdin
    int i = 0;

    while (read(0, &buffer[i], 1) > 0) {
        if (buffer[i] == '\n') 
        {
            buffer[i] = '\0';  // kết thúc chuỗi tại vị trí ngắt dòng

            cmd_argv[base_argc] = buffer; // thêm dòng nhập vào cuối mảng tham số
            cmd_argv[base_argc + 1] = 0; // kết thúc mảng tham số với NULL vì exec yêu cầu mảng tham số phải kết thúc bằng NULL

            if (fork() == 0) { 
                exec(cmd_argv[0], cmd_argv);

                fprintf(2, "Failed to execute command: %s\n", cmd_argv[0]);
                exit(1);
            }
            wait(0); 
            i = 0; // đặt lại chỉ số để đọc dòng tiếp theo
        } else {
            i++; // tiếp tục đọc ký tự tiếp theo nếu chưa gặp ngắt dòng
        }
    }
    exit(0);
}