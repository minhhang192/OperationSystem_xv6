#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// Hàm chạy lệnh với các tham số đã chuẩn bị sẵn
void run(char *cmd_argv[]) {
    if (fork() == 0) {
        exec(cmd_argv[0], cmd_argv);
        fprintf(2, "exec failed\n");
        exit(1);
    }
    wait(0);
}


int main(int argc, char *argv[]) {
    // kiểm tra số lượng tham số
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }

    char *cmd_argv[MAXARG]; // mảng chứa các tham số cho lệnh sẽ thực thi
    int base_argc = 0; // số lượng tham số ban đầu 

    // lấy các tham số từ dòng lệnh và lưu vào cmd_argv
    for (int i = 1; i < argc; i++) {
        cmd_argv[base_argc++] = argv[i];
    }

    char buffer[512]; // mảng để lưu trữ dòng nhập từ stdin được kết thúc bởi "\n"
    int i = 0;
    int n = 0;

    while ((n = read(0, &buffer[i], 1)) > 0) {

        // kiểm tra nếu dòng nhập quá dài sẽ in ra lỗi và thoát chương trình
        if (i >= sizeof(buffer) - 1) {
            fprintf(2, "line too long\n");
            exit(1);
        }

        if (buffer[i] == '\n') 
        {
            buffer[i] = '\0';  // kết thúc chuỗi tại vị trí ngắt dòng

            cmd_argv[base_argc] = buffer; // thêm dòng vừa đọc (từ stdin) làm tham số cuối cho lệnh
            cmd_argv[base_argc + 1] = 0; // kết thúc argv bằng NULL theo yêu cầu của exec

            
            run(cmd_argv);

            i = 0; // đặt lại chỉ số để đọc dòng tiếp theo
        } else {
            i++; // tiếp tục đọc ký tự tiếp theo nếu chưa gặp ngắt dòng
        }
    }
    // xử lý dòng cuối nếu không kết thúc bằng ngắt dòng
    if (i > 0) {
        buffer[i] = '\0';

        cmd_argv[base_argc] = buffer;
        cmd_argv[base_argc + 1] = 0;

        run(cmd_argv);  
    }

    exit(0);
}