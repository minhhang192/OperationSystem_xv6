#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "user/procinfo.h"

extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

<<<<<<< HEAD
// /// @brief Get process information base on PID provided by User
// /// @param pid Process ID
// /// @param info_addr Address to store process information
// /// @return 0 on success, -1 on failure
// uint64 sys_getproc(void){
//   int pid; // Process ID
//   uint64 info_addr; // Address to store process information
//   struct procinfo info; // Struct to hold process information

//   argint(0, &pid); // Get the PID address provided by User
//   // Khi bạn truyền một struct hoặc một mảng từ User vào Kernel 
//   // bạn không truyền cả khối dữ liệu đó được. Bạn chỉ truyền địa chỉ của con trỏ của khối dữ liệu đó
//   // Kernel sẽ sử dụng địa chỉ đó để copy dữ liệu từ User space vào Kernel space hoặc ngược lại. bằng copyin hoặc copyout
//   argaddr(1, &info_addr); // Struct procinfo address provided by user will be stored in info_addr

//   struct proc *p = myproc(); // Get the current process
//   struct proc *target = 0; // Pointer to the target process whose information we want to get

//   //acquire(&p -> lock);
//   // Giữ lock của process hiện tại để đảm bảo an toàn khi truy cập thông tin process
//   // Khi một CPU gọi lệnh acquire, nó sẽ kiểm tra xem lock đã được giữ bởi một CPU khác chưa. 
//   //Nếu lock đã được giữ, CPU sẽ bị chặn (blocked) cho đến khi lock được giải phóng. 
//   //Khi lock được giải phóng, một trong số các CPU đang chờ sẽ được chọn để tiếp tục và giữ lock đó. 
//   //Điều này đảm bảo rằng chỉ có một CPU có thể giữ lock tại một thời điểm, 
//   //giúp tránh xung đột và đảm bảo tính nhất quán của dữ liệu khi nhiều CPU truy cập cùng một tài nguyên. 

//   // Tìm process có PID bằng với PID mà User truyền lên trong hệ thống
//   // Duyệt qua danh sách process của hệ thống để tìm process có PID bằng với PID mà User truyền lên
//   // struct proc *pp = p: khởi tạo con trỏ pp trỏ đến process hiện tại p.
//   // pp != 0: điều kiện tiếp tục vòng lặp, đảm bảo rằng pp không phải là con trỏ null (tức là vẫn còn process để kiểm tra).
//   // pp = pp->parent: sau mỗi lần lặp, con trỏ pp sẽ nhảy lên một nấc cao hơn trong cây process, 
//   // tức là sẽ kiểm tra process cha của process hiện tại.
//   for(struct proc *pp = p; pp < &proc[NPROC]; pp = pp -> parent){
//     acquire(&pp -> lock); // Giữ lock của process pp để đảm bảo an toàn khi truy cập thông tin process pp
//     if(pp -> pid == pid){ // Nếu tìm thấy process có PID bằng với PID mà User truyền lên
//       target = pp; // Gán con trỏ target trỏ đến process đó
//       // Nếu tìm thấy process có PID bằng với PID mà User truyền lên,
//       // Gán thông tin của process đó vào struct procinfo info
//       info.pid = target -> pid;
//       info.ppid = target -> parent ? target -> parent -> pid : -1;
//       info.state = target -> state;
//       info.sz = target -> sz;
//       safestrcpy(info.name, target -> name, sizeof(info.name)); // Sao chép tên process vào struct procinfo, đảm bảo không tràn bộ nhớ
//       break; 
//     }
//     release(&pp -> lock); // Giải phóng lock của process pp sau khi đã lấy thông tin
//   }
//   if(target == 0){
//     //release(&p -> lock);
//     return -1;
//   }
 
//   //release(&p -> lock);

//   // p -> pagetable: bảng trang của process hiện tại, được sử dụng để xác định cách ánh xạ địa chỉ ảo sang địa chỉ vật lý.
//   // info_addr: địa chỉ trong không gian người dùng nơi struct procinfo sẽ được sao chép đến.
//   // (char *)&info: địa chỉ của struct procinfo trong kernel, được chuyển đổi thành con trỏ char để phù hợp với kiểu dữ liệu mà copyout yêu cầu.
//   // sizeof(info): kích thước của struct procinfo, được sử dụng để xác định lượng dữ liệu cần sao chép 
//   // đảm bảo rằng không có tràn bộ nhớ khi sao chép dữ liệu từ kernel space sang user space.
//   if(copyout(p -> pagetable, info_addr, (char *)&info, sizeof(info)) < 0){
//     return -1; // Nếu có lỗi khi sao chép thông tin về user space, trả về -1
//   }
  
//   return 0;
// }

uint64
sys_getproc(void)
{
  int pid; // Process ID 
  uint64 info_addr; // Address to store process information provided by User
  struct procinfo info; // Cấu trúc tạm lưu trên kernel stack 
  struct proc *p; // Con trỏ để duyệt qua bảng tiến trình của hệ thống

  int found = 0; // sentinel để kiểm tra xem đã tìm thấy process có PID yêu cầu hay chưa

  // 1. Lấy các đối số từ không gian người dùng 
  argint(0, &pid); // Lấy PID từ đối số đầu tiên mà User truyền lên
  argaddr(1, &info_addr);// Lấy địa chỉ trong không gian người dùng nơi struct procinfo sẽ được sao chép đến từ đối số thứ hai mà User truyền lên

  // 2. Duyệt qua toàn bộ bảng tiến trình hệ thống thay vì chỉ duyệt cây cha con
  // p = proc: khởi tạo con trỏ p trỏ đến đầu của bảng tiến trình proc.
  // p < &proc[NPROC]: điều kiện tiếp tục vòng lặp, đảm bảo rằng p vẫn còn trỏ đến một tiến trình hợp lệ trong bảng tiến trình (chưa vượt quá giới hạn NPROC).
  // p++: sau mỗi lần lặp, con trỏ p sẽ di chuyển đến process tiếp theo trong bảng tiến trình, cho phép kiểm tra tất cả các tiến trình có trong hệ thống.
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock); // Bảo vệ các trường state, sz, name khỏi race condition 
    
    // Kiểm tra PID và đảm bảo tiến trình không phải ở trạng thái UNUSED
    if(p->state != UNUSED && p->pid == pid) {
      found = 1;

      // 3. Trích xuất thông tin vào cấu trúc tạm trong kernel 
      info.pid = p->pid;
      // PPID: Nếu không có cha (như init), trả về -1 để an toàn 
      info.ppid = p->parent ? p->parent->pid : -1;
      info.state = p->state;
      info.sz = p->sz;
      safestrcpy(info.name, p->name, sizeof(info.name));

      release(&p->lock); // Giải phóng ngay sau khi copy xong dữ liệu nhạy cảm
      break;
    }
    release(&p->lock);
  }

  if(!found)
    return -1; // Không tìm thấy PID yêu cầu

  // 4. Sử dụng copyout để chuyển dữ liệu sang User Space một cách an toàn 
  // Hàm này dò bảng trang để đảm bảo info_addr là hợp lệ 
  // p -> pagetable: bảng trang của process hiện tại, được sử dụng để xác định cách ánh xạ virtual address in user space sang physical address in kernel.
  // info_addr: địa chỉ trong không gian người dùng nơi struct procinfo sẽ được sao chép đến.
  // (char *)&info: địa chỉ của struct procinfo trong kernel, được chuyển đổi thành con trỏ char để phù hợp với kiểu dữ liệu mà copyout yêu cầu.
  // sizeof(info): kích thước của struct procinfo, được sử dụng để xác định lượng dữ liệu cần sao chép 
  // đảm bảo rằng không có tràn bộ nhớ khi sao chép dữ liệu từ kernel space sang user space.
  struct proc *curproc = myproc();
  if(copyout(curproc->pagetable, info_addr, (char *)&info, sizeof(info)) < 0)
    return -1;

=======
uint64
sys_trace(void)
{
  int mask;
  argint(0, &mask);
  myproc()->tracemask = mask;
>>>>>>> 7a4e9dce175d939b54b60317b70e70eb961ffe8f
  return 0;
}