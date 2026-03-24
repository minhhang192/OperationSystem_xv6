#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG]; // mang chua doi so moi cho cac lenh se chay

  //kiem tra tham so dau vao
  //phai co it nhat 2 doi so va doi so dau tien phai la mask
  if(argc < 2 || (argv[1][0] < '0' || argv[1][0] > '9')){
    fprintf(2, "usage: trace mask command\n");
    exit(1);
  }
  // goi syscall trace() neu that bai thi bao loi, thoat
  if(trace(atoi(argv[1])) < 0){
    fprintf(2, "trace: trace failed\n");
    exit(1);
  }

  //bo qua 2 doi so dau tien (trace va mask)
  for(i = 2; i < argc && i < MAXARG; i++){
    nargv[i-2] = argv[i];
  }
  nargv[argc-2] = 0;   // null terminate

  exec(argv[2], nargv);   // chạy chương trình được trace
  fprintf(2, "trace: exec failed\n");
  exit(1);
}