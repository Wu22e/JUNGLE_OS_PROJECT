#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "userprog/process.h"

void syscall_entry(void);
void syscall_handler(struct intr_frame*);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init(void) {
    lock_init(&filesys_lock);
    write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48 |
        ((uint64_t)SEL_KCSEG) << 32);
    write_msr(MSR_LSTAR, (uint64_t)syscall_entry);

    /* The interrupt service rountine should not serve any interrupts
     * until the syscall_entry swaps the userland stack to the kernel
     * mode stack. Therefore, we masked the FLAG_FL. */
    write_msr(MSR_SYSCALL_MASK,
        FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

void check_address(void* addr) {
    if (!is_user_vaddr(addr))
        sys_exit(-1);
}

// void get_argument(void* rsp, int* arg, int count) {
//     for(int i = 0; i < count; i++)
//         *(arg + i) = *((int *)rsp + i + 1);
// } 


/* The main system call interface */
void
syscall_handler(struct intr_frame* f UNUSED) {
    // TODO: Your implementation goes here.
    // printf("system call!\n");

    check_address(f->rsp);
    // int number = f->rsp; // number = f->R.rax 이렇게 하나 똑같은 거겟지?
    switch (f->R.rax)
    {
    case SYS_HALT:
        sys_halt();
        break;

    case SYS_EXIT:
        // get_argument(f->rsp, args, 1);
        sys_exit((int)f->R.rdi);
        break;

    case SYS_FORK:
        // get_argument(f->rsp, args, 1);
        f->R.rax = sys_fork((const char*)f->R.rdi);
        // arg[0] = fork(f->R.rdi); // 이렇게 하면 안되겟지?
        break;

    case SYS_EXEC:
        // get_argument(f->rsp, args, 1);
        f->R.rax = sys_exec((const char*)f->R.rdi);
        break;

    case SYS_WAIT:
        // get_argument(f->rsp, args, 1);
        f->R.rax = sys_wait((pid_t)f->R.rdi);
        break;

    case SYS_CREATE:
        f->R.rax = sys_create((const char*)f->R.rdi, (unsigned)f->R.rsi);
        break;

    case SYS_REMOVE:
        f->R.rax = sys_remove((const char*)f->R.rdi);
        break;

    case SYS_OPEN:
        f->R.rax = sys_open((const char*)f->R.rdi);
        break;

    case SYS_FILESIZE:
        f->R.rax = sys_filesize((int)f->R.rdi);
        break;

    case SYS_READ:
        f->R.rax = sys_read((int)f->R.rdi,(void*)f->R.rsi,(unsigned)f->R.rdx);
        break;

    case SYS_WRITE:
        f->R.rax = sys_write((int)f->R.rdi,(void*)f->R.rsi,(unsigned)f->R.rdx);
        break;

    case SYS_SEEK:
        break;

    case SYS_TELL:
        break;

    case SYS_CLOSE:
        break;

    default:
        thread_exit();
        break;
    }
}

void sys_halt(void) {
    power_off();
}

void sys_exit(int status) {
    struct thread* cur = thread_current();
    cur->exit_status = status;
    cur->process_exit = 1;
    printf("%s: exit(%d)\n", cur->name, cur->exit_status);
    thread_exit();
}

pid_t sys_fork(const char* thread_name) {
    struct thread* t = thread_current();
    process_fork(thread_name, NULL);
}

int sys_exec(const char* file) {
    tid_t child_tid = process_create_initd(file);
    struct thread* child_thread = get_child_process(child_tid);
    sema_down(&child_thread->semaphore_load);
    if (child_thread->process_load == 0)
        return -1;
    else
        return child_tid;
}

int sys_wait(pid_t pid) {
    return process_wait(pid);
}

bool sys_create(const char* file, unsigned initial_size) {
    if (file == NULL) { sys_exit(-1); }
    return filesys_create(file, initial_size);
}

bool sys_remove(const char* file) {
    return filesys_remove(file);
}

int sys_open(const char* file) {
    /* 파일을 open */
    /* 해당 파일 객체에 파일 디스크립터 부여 */
    /* 파일 디스크립터 리턴 */
    /* 해당 파일이 존재하지 않으면 -1 리턴 */
    if (file == NULL) { sys_exit(-1); }
    int temp = process_add_file(filesys_open(file));
    // printf("------------>temp is = %d<--------------\n",temp);
    return temp;
    //! process_add_file에서 fd가 없을때 NULL까지 반환시켜서 괜찮음
}

int sys_filesize(int fd) {
    /* 파일 디스크립터를 이용하여 파일 객체 검색 */
    /* 해당 파일의 길이를 리턴 */
    /* 해당 파일이 존재하지 않으면 -1 리턴 */
    struct file* f = process_get_file(fd);
    if (f == NULL) return -1;
    return file_length(f);
}

int sys_read(int fd, void* buffer, unsigned size)
{
    /* 파일에 동시 접근이 일어날 수 있으므로 Lock 사용 */
    /* 파일 디스크립터를 이용하여 파일 객체 검색 */
    /* 파일 디스크립터가 0일 경우 키보드에 입력을 버퍼에 저장 후
       버퍼의 저장한 크기를 리턴 (input_getc() 이용) */
    /* 파일 디스크립터가 0이 아닐 경우 파일의 데이터를 크기만큼 저장 후 
       읽은 바이트 수를 리턴 */

    struct file *f;
    lock_acquire(&filesys_lock);
    int bytes = size;
    f = process_get_file(fd);
    if(fd == 0){
        // while(bytes){
            bytes = input_getc();
        // }
    }
    else{
        bytes = file_read(f,buffer,size); 
    }
    lock_release(&filesys_lock);
    return bytes;
}

int sys_write(int fd, void *buffer, unsigned size)
{
    /* 파일에 동시 접근이 일어날 수 있으므로 Lock 사용 */
    /* 파일 디스크립터를 이용하여 파일 객체 검색 */
    /* 파일 디스크립터가 1일 경우 버퍼에 저장된 값을 화면에 출력
       후 버퍼의 크기 리턴 (putbuf() 이용) */
    /* 파일 디스크립터가 1이 아닐 경우 버퍼에 저장된 데이터를 크기
       만큼 파일에 기록후 기록한 바이트 수를 리턴 */
    struct file *f;
    lock_acquire(&filesys_lock);
    int bytes;
    f = process_get_file(fd);
    if(fd == 1){
        putbuf((const char*)buffer, size);
        bytes = size;
    }
    else{
        bytes = file_write(f,buffer,size); 
    }
    lock_release(&filesys_lock);
    return bytes;
}
