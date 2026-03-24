#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

//argc (Argument Count): Số lượng tham số.
//argv (Argument Vector): Mảng chứa các tham số. 

int main(int argc, char *argv[]) {
    // Bước 1: Kiểm tra xem người dùng đã nhập đủ tham số chưa (phải có ít nhất 2 tham số: "xargs" + lệnh muốn chạy)
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }

    // Bước 2: Tạo một mảng cmd_argv để chứa tên lệnh và tất cả tham số gốc + tham số mới ghép từ stdin.
    char *cmd_argv[MAXARG]; // cmd_argv sẽ chứa tên lệnh và tất cả tham số gốc + tham số mới ghép từ stdin.
    int base_argc = 0;     // base_argc là số lượng tham số gốc (bao gồm tên lệnh) mà người dùng đã nhập khi gọi xargs

    // Lấy lệnh người dùng muốn chạy (và các tham số gốc bỏ vào mảng cmd_argv) 
    for (int i = 1; i < argc; i++) {
        cmd_argv[base_argc++] = argv[i];
    }

    // Bước 3: Chuẩn bị bộ đệm để ghép các tham số mới từ stdin
    char buffer[512];        // bộ đệm hứng từng chữ cái truyền vào
    int buffer_idx = 0;      // vị trí để nhét chữ cái tiếp theo vào buffer
    char c;              

    int curr_argc = base_argc; // số lượng tham số hiện tại (bắt đầu bằng số lượng tham số gốc)
    char *curr_arg = buffer; // trỏ vào đầu chữ đang được ghép trong bộ đệm

    // Bước 4: Đọc từng ký tự một từ stdin (bàn phím hoặc lệnh đứng trước dấu '|') để ghép thành các tham số mới
    while (read(0, &c, 1) == 1) {
        
        // Luồng 1: Nếu ký tự vừa đọc được là dấu cách hoặc dấu xuống dòng => ghép xong một từ hoàn chỉnh
        if (c == ' ' || c == '\n') {
            
            // đánh dấu kết thúc một từ hoàn chỉnh
            buffer[buffer_idx++] = '\0'; 

            // tránh trường hợp gõ 2 dấu cách liên tiếp nên nếu rỗng thì không đưa vào danh sách tham số
            if (*curr_arg != '\0') {
                cmd_argv[curr_argc++] = curr_arg;
            }
            
            // hết dòng thì đem đi chạy lệnh
            if (c == '\n') {
                // exec yêu cầu phần tử cuối cùng của mảng tham số phải là 0 (NULL)
                cmd_argv[curr_argc] = 0; 

                // TẠO TIẾN TRÌNH CON ĐỂ CHẠY LỆNH (Tránh làm chết tiến trình xargs hiện tại)
                if (fork() == 0) {
                    // LUỒNG CỦA CON: Gọi hàm exec
                    // tham số 1: tên lệnh (cmd_argv[0])
                    // tham số 2: toàn bộ mảng tham số (cmd_argv)
                    exec(cmd_argv[0], cmd_argv);
                    
                    // nếu lệnh không tồn tại thì in ra lỗi
                    fprintf(2, "exec failed\n");
                    exit(1);
                } else {
                    // LUỒNG CỦA CHA: chờ con chạy xong 
                    wait(0);
                }

                // chạy xong thì reset để cho dòng mới
                buffer_idx = 0;                  
                curr_argc = base_argc;        
                curr_arg = buffer;               
            } else {
                // nếu chưa hết dòng thì tiếp tục ghép từ mới vào bộ đệm
                curr_arg = &buffer[buffer_idx];
            }
        } 
        // Luồng 2: nếu không phải cả 2 thì là kí tự bình thường thì cứ ghép vào bộ đệm để tạo thành từ hoàn chỉnh
        else {
            buffer[buffer_idx++] = c;
        }
    }
    exit(0);
}