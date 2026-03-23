#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Tham số: 
// path: Nơi mà chương trình đang đứng 
// depth: Độ sâu để in ra các khoảng trắng cho đẹp
void tree(char *path, int depth) {
    char buffer[512]; // Mảng 512 bytes chứa đường dẫn của các file con bên trong
    char *pointer;       // Con trỏ dùng để ghép chuỗi tên file
    int file_des;        // File Descriptor (con trỏ quản lý file/thư mục)
    
    struct dirent dir_entry; // Cấu trúc chứa thông tin của file/thư mục (directory entry)
    struct stat st;   // Cấu trúc chứa chi tiết xem nó là file hay thư mục (stat)

    // Bước 1: mở thư mục hiện tại (path) để bắt đầu đọc thông tin bên trong
    if((file_des = open(path, 0)) < 0){
        fprintf(2, "tree: cannot open %s\n", path);
        exit(1); 
    }

    // Bước 2: Lấy thông tin của thư mục hiện tại để biết nó có phải là thư mục hay không
    if(fstat(file_des, &st) < 0){
        fprintf(2, "tree: cannot stat %s\n", path);
        close(file_des);
        return;
    }

    // Bước 3: Nếu nó là thư mục thì mới bắt đầu đọc thông tin của các file con bên trong
    if(st.type == T_DIR){
        
        // Nếu đường dẫn lồng nhau quá sâu (> 512 bytes) sẽ làm tràn bộ nhớ
        // DIRSIZ là kích thước cố định của trường name trong struct dirent (14 bytes)
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buffer)){
            printf("tree: path too long\n");
            close(file_des);    
            return;
        }

        // Tạo tiền đề cho đường dẫn con: vd "a" => "a/"
        strcpy(buffer, path);
        pointer = buffer + strlen(buffer);
        *pointer++ = '/'; // Chèn dấu '/' vào cuối và nhích con trỏ p lên 1 nấc

        // Bước 4: Đọc thông tin của từng file con bên trong thư mục hiện tại
        // dir_entry sẽ chứa thông tin của từng file con (như tên, số inum...)
        while(read(file_des, &dir_entry, sizeof(dir_entry)) == sizeof(dir_entry)){
            // inum == 0 nghĩa là file này đã bị user xóa đi rồi, bỏ qua
            if(dir_entry.inum == 0)
                continue;

            // Bỏ qua "." (Thư mục hiện hành) và ".." (Thư mục cha)
            // Nếu không có bước này, đệ quy sẽ chạy lặp vòng tròn vô tận
            if(strcmp(dir_entry.name, ".") == 0 || strcmp(dir_entry.name, "..") == 0)
                continue;

            // Dán tên file con vào sau dấu '/'
            // Vd: Đang có "a/" + tên file con là "b" => Thành "a/b"
            memmove(pointer, dir_entry.name, DIRSIZ);
            pointer[DIRSIZ] = 0; // Chốt đuôi chuỗi bằng ký tự NULL

            struct stat st_child;
            // Lấy thông tin (stat) của file con vừa ghép được
            if(stat(buffer, &st_child) < 0){
                fprintf(2, "tree: cannot stat %s\n", buffer);
                continue;
            }

            for(int i = 0; i < depth; i++) {
                printf("  "); // In 2 dấu cách cho mỗi cấp độ
            }

            // Bước 5: Kiểm tra xem file con này là thư mục hay file bình thường
            // T_DIR nghĩa là nó là một thư mục, còn nếu không phải T_DIR thì coi như là file bình thường
            if(st_child.type == T_DIR){
                // Nếu là thư mục: In tên kèm dấu '/', sau đó gọi đệ quy để đi sâu vào trong
                printf("%s/\n", dir_entry.name);
                
                // Gọi đệ quy: Truyền đường dẫn 'buffer' mới vào, và tăng độ sâu (depth) lên 1 cấp
                // depth tăng lên để khi in ra các file con bên trong thư mục này sẽ có thêm khoảng trắng thụt lề
                tree(buffer, depth + 1);
            } else {
                // Nếu chỉ là file bình thường: In cái tên ra là xong
                printf("%s\n", dir_entry.name);
            }
        }
    }
    // Đóng thư mục hiện tại sau khi đã quét xong
    close(file_des);
}

int main(int argc, char *argv[]) {
    // Nếu gõ lệnh 'tree' mà không truyền tham số => Mặc định là thư mục hiện tại "."
    char *path = ".";

    // Nếu có truyền tham số (vd: 'tree a/b') => Lấy tham số ở vị trí số 1 (argv[1])
    if(argc > 1){
        path = argv[1];
    }

    struct stat st;
    if (stat(path, &st) < 0) {
        fprintf(2, "tree: cannot stat %s\n", path);
        exit(1);
    }

    // In cái tên của thư mục/file gốc ra đầu tiên (kèm dấu '/' nếu nó là thư mục)
    if (st.type == T_DIR) {
        printf("%s/\n", path);
    } else {
        printf("%s\n", path);
    }

    // Nếu cái đỉnh đó thực sự là thư mục, thì kích hoạt hàm đệ quy để quét các nhánh
    // Bắt đầu với mức độ thụt lề (depth) là 1
    if (st.type == T_DIR) {
        tree(path, 1);
    }

    exit(0);
}