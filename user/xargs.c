#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

//argc (Argument Count): Số lượng tham số.
//argv (Argument Vector): Mảng chứa các tham số. 

int main(int argc, char *argv[]) {
    // BƯỚC 1: KIỂM TRA ĐẦU VÀO
    // Phải có ít nhất 2 tham số. Ví dụ: gọi 'xargs echo' thì argc là 2.
    // Nếu chỉ gọi mỗi 'xargs' (argc < 2) thì báo lỗi và thoát.
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }

    // BƯỚC 2: CHUẨN BỊ MẢNG CHỨA CÁC THAM SỐ ĐỂ CHẠY LỆNH MỚI
    // MAXARG là giới hạn số lượng tham số tối đa của hệ điều hành xv6
    char *cmd_argv[MAXARG]; // cmd_argv sẽ chứa tên lệnh và tất cả tham số gốc + tham số mới ghép từ stdin.
    int base_argc = 0;     // base_argc là số lượng tham số gốc (bao gồm tên lệnh) mà người dùng đã nhập khi gọi xargs

    // Lấy lệnh người dùng muốn chạy (và các tham số gốc bỏ vào mảng cmd_argv
    for (int i = 1; i < argc; i++) {
        cmd_argv[base_argc++] = argv[i]; // Bắt đầu từ argv[1] vì argv[0] là "xargs" rồi
    }

    // BƯỚC 3: CHUẨN BỊ BỘ NHỚ ĐỂ ĐỌC DỮ LIỆU TỪ STANDARD INPUT
    char buffer[512];        // Bộ đệm 512 bytes để hứng từng chữ cái người dùng truyền vào
    int buffer_idx = 0;      // Vị trí index để nhét chữ cái tiếp theo vào 'buffer'
    char c;               // Biến hứng từng ký tự (A, B, C, dấu cách, dấu xuống dòng...)

    int curr_argc = base_argc; // Số lượng tham số hiện tại (bắt đầu bằng số lượng tham số gốc)
    char *curr_arg = buffer; // Trỏ vào đầu chữ đang được ghép trong bộ đệm

    // BƯỚC 4: VÒNG LẶP ĐỌC LIÊN TỤC TỪ STDIN (File descriptor 0)
    // read(0, &c, 1) sẽ đọc 1 ký tự từ bàn phím hoặc từ lệnh đứng trước dấu '|', và lưu vào biến 'c'.
    while (read(0, &c, 1) == 1) {
        
        // LUỒNG 1: NẾU GẶP DẤU CÁCH HOẶC XUỐNG DÒNG => ĐÃ GHÉP XONG 1 TỪ
        if (c == ' ' || c == '\n') {
            
            // Chốt đuôi chữ vừa ghép bằng ký tự '\0' (để C hiểu đây là kết thúc chuỗi)
            buffer[buffer_idx++] = '\0'; 

            // Kiểm tra xem chữ vừa ghép có bị rỗng không (tránh trường hợp gõ 2 dấu cách liên tiếp)
            if (*curr_arg != '\0') {
                // Nhét chữ vừa ghép xong vào danh sách tham số để chuẩn bị chạy
                cmd_argv[curr_argc++] = curr_arg;
            }

            // Nếu là dấu xuống dòng => Đã hết 1 dòng, đem các tham số đi chạy lệnh thôi!
            if (c == '\n') {
                // Quy tắc của hàm exec: phần tử cuối cùng của mảng tham số phải là 0 (NULL)
                cmd_argv[curr_argc] = 0; 

                // TẠO TIẾN TRÌNH CON ĐỂ CHẠY LỆNH (Tránh làm chết tiến trình xargs hiện tại)
                if (fork() == 0) {
                    // LUỒNG CỦA CON: Gọi hàm exec để "biến hình" thành lệnh mới
                    // Tham số 1: Tên lệnh (cmd_argv[0])
                    // Tham số 2: Toàn bộ mảng tham số (cmd_argv)
                    exec(cmd_argv[0], cmd_argv);
                    
                    // Nếu exec chạy thành công, nó sẽ không bao giờ chạy xuống dòng dưới này.
                    // Nên nếu chạy tới đây tức là lỗi (lệnh không tồn tại).
                    fprintf(2, "exec failed\n");
                    exit(1);
                } else {
                    // LUỒNG CỦA CHA: Đứng chờ tiến trình con chạy xong thì mới làm tiếp
                    wait(0);
                }

                // CHẠY XONG RỒI => RESET LẠI MỌI THỨ ĐỂ ĐỌC DÒNG TIẾP THEO
                buffer_idx = 0;                  // Xóa trắng bộ đệm
                curr_argc = base_argc;        // Xóa các tham số cũ, chỉ giữ lại lệnh gốc
                curr_arg = buffer;               // Quay đầu con trỏ về vị trí xuất phát
            } else {
                // Nếu chỉ là dấu cách (chưa xuống dòng) => Trỏ 'curr_arg' tới vị trí chữ cái tiếp theo
                curr_arg = &buffer[buffer_idx];
            }
        } 
        // LUỒNG 2: NẾU LÀ CHỮ CÁI BÌNH THƯỜNG (A, B, C, 1, 2, 3...)
        else {
            // Cứ nhét chữ cái đó vào bộ đệm tạm để ghép từ từ thành một từ hoàn chỉnh
            buffer[buffer_idx++] = c;
        }
    } // Hết vòng lặp (Hết dữ liệu đầu vào)

    exit(0); // Kết thúc xargs thành công
}