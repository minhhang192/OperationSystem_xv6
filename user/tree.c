#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// path: Nơi mà chương trình đang đứng 
// depth: Độ sâu để in ra các khoảng trắng cho đẹp
void tree(char *path, int depth) {
    char buffer[512]; // mảng chứa đường dẫn của các file con bên trong
    char *pointer;       // con trỏ dùng để ghép chuỗi tên file
    int file_des;        // File Descriptor (con trỏ quản lý file/thư mục)
    
    struct dirent dir_entry; // Cấu trúc chứa thông tin của file/thư mục (directory entry)
    struct stat st;   // Cấu trúc chứa chi tiết xem nó là file hay thư mục (stat)

    // Bước 1: mở thư mục hiện tại (path) để bắt đầu đọc thông tin bên trong
    if((file_des = open(path, 0)) < 0){
        fprintf(2, "tree: cannot open %s\n", path);
        exit(1); 
    }

    // Bước 2: kiểm tra thông tin của thư mục hiện tại để biết nó có phải là thư mục hay không
    if(fstat(file_des, &st) < 0){
        fprintf(2, "tree: cannot stat %s\n", path);
        close(file_des);
        return;
    }

    // Bước 3: nếu nó là thư mục thì mới bắt đầu đọc thông tin của các file con bên trong
    if(st.type == T_DIR){
        
        // nếu đường dẫn lồng nhau quá sâu sẽ làm tràn bộ nhớ
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buffer)){
            printf("tree: path too long\n");
            close(file_des);    
            return;
        }

        // vd "a" => "a/"
        strcpy(buffer, path);
        pointer = buffer + strlen(buffer);
        *pointer++ = '/'; // Chèn dấu '/' vào cuối và nhích con trỏ p lên 1 nấc

        // Bước 4: đọc thông tin của từng file con bên trong thư mục hiện tại
        // dir_entry sẽ chứa thông tin của từng file con (như tên, số inum...)
        while(read(file_des, &dir_entry, sizeof(dir_entry)) == sizeof(dir_entry)){
            // inum == 0 nghĩa là file này đã bị xóa
            if(dir_entry.inum == 0)
                continue;

            // bỏ qua "." (Thư mục hiện hành) và ".." (Thư mục cha)
            if(strcmp(dir_entry.name, ".") == 0 || strcmp(dir_entry.name, "..") == 0)
                continue;

            // dán tên file con vào sau dấu '/'
            // vd: a/ + tên file con là "b" thành "a/b"
            memmove(pointer, dir_entry.name, DIRSIZ);
            pointer[DIRSIZ] = 0; // đánh dấu kết thúc chuỗi sau khi dán tên file con vào

            struct stat st_child;
            // lấy thông tin (stat) của file con vừa ghép được
            if(stat(buffer, &st_child) < 0){
                fprintf(2, "tree: cannot stat %s\n", buffer);
                continue;
            }

            for(int i = 0; i < depth; i++) {
                printf("  "); // in 2 dấu cách cho mỗi cấp thư mục
            }

            // Bước 5: kiểm tra xem file con này là thư mục hay file bình thường
            if(st_child.type == T_DIR){
                // nếu là thư mục thì in tên kèm dấu '/',
                printf("%s/\n", dir_entry.name);
                
                // gọi đệ quy và truyền vào đường dẫn của file con đó và độ sâu tăng lên 1 (depth + 1)
                // depth tăng lên để khi in ra các file con bên trong thư mục này sẽ có thêm khoảng trắng thụt lề
                tree(buffer, depth + 1);
            } else {
                // file bình thường thì chỉ in tên
                printf("%s\n", dir_entry.name);
            }
        }
    }
    close(file_des);
}

int main(int argc, char *argv[]) {
    // mặc định nếu không truyền tham số nào thì sẽ in ra cây thư mục từ thư mục hiện tại (".")
    char *path = ".";

    if(argc > 1){
        path = argv[1];
    }

    struct stat st;
    if (stat(path, &st) < 0) {
        fprintf(2, "tree: cannot stat %s\n", path);
        exit(1);
    }

    if (st.type == T_DIR) {
        printf("%s/\n", path);
    } else {
        printf("%s\n", path);
    }

    // nếu là thư mục thì mới gọi hàm tree để in ra cây thư mục bên trong
    if (st.type == T_DIR) {
        tree(path, 1);
    }

    exit(0);
}