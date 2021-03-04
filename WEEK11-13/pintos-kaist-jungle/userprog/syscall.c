#include "userprog/syscall.h"

#include <stdio.h>
#include <syscall-nr.h>

#include "filesys/file.h"
#include "filesys/filesys.h"
#include "intrinsic.h"
#include "lib/kernel/console.h"
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/loader.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "userprog/gdt.h"
#include "userprog/process.h"
//! 추가 
// #include "filesys/inode.h"


void syscall_entry(void);
void syscall_handler(struct intr_frame *);

void syscall_init(void);

void halt(void) NO_RETURN;
void exit(int status) NO_RETURN;
pid_t fork(const char *thread_name, struct intr_frame *f);
int exec(const char *file);
int wait(pid_t);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned length);
int write(int fd, const void *buffer, unsigned length);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
//! 2/25 추가 !
void *mmap(void *addr, size_t length, int writable, int fd, off_t offset);
void munmap(void *addr);
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

struct file {
    struct inode *inode; /* File's inode. */
    off_t pos;           /* Current position. */
    bool deny_write;     /* Has file_deny_write() been called? */
    struct lock file_lock;
};

void syscall_init(void) {
    // puts("------------------ userprog/syscall.c:syscall_init       ------------------");
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

/* The main system call interface */
void syscall_handler(struct intr_frame *f UNUSED) {
    // puts("------------------ userprog/syscall.c:syscall_handler    ------------------");
    // check_address(sp);
    int syscall_num = (f->R.rax);

    // printf("------syscall_num: %d ------\n", syscall_num);
    // printf("heyhey\n");
    //! 2/25 변경! (mmap 인자가 많아져서 인자를 추가해줌)
    uint64_t argument[6] = {f->R.rdi, f->R.rsi, f->R.rdx, f->R.r10, f->R.r8, f->R.r9};

    //struct intr_frame *cur_if = &thread_current()->tf;
    thread_current()->rsp_stack = f->rsp; //! 3/2 추가 (try_handle_fault 에서 커널모드일때의 유저 rsp 스택포인터를 가져와야 함)

    switch (syscall_num) {
        case SYS_HALT:
            halt();
            break;
        case SYS_EXIT:
            // puts("------------------ userprog/syscall.c:hand-exit               ------------------");
            exit((int)argument[0]);
            break;
        case SYS_FORK:
            f->R.rax = fork((char *)argument[0], f);
            break;
        case SYS_EXEC:
            f->R.rax = exec((char *)argument[0]);
            break;
        case SYS_WAIT:
            // puts("------------------ userprog/syscall.c:hand-wait               ------------------");
            f->R.rax = wait((int)argument[0]);
            break;
        case SYS_CREATE:
            f->R.rax = create((char *)argument[0], (unsigned)argument[1]);
            break;
        case SYS_REMOVE:
            f->R.rax = remove((char *)argument[0]);
            break;
        case SYS_OPEN:
            f->R.rax = open((char *)argument[0]);
            break;
        case SYS_FILESIZE:
            f->R.rax = filesize((int)argument[0]);
            break;
        case SYS_READ:
            f->R.rax = read((int)argument[0], (void *)argument[1], (unsigned)argument[2]);
            break;
        case SYS_WRITE:
            // puts("*************************** userprog/syscall.c:handler:write    ***************************");
            f->R.rax = write((int)argument[0], (const void *)argument[1], (unsigned int)argument[2]);
            break;
        case SYS_SEEK:
            seek((int)argument[0], (unsigned)argument[1]);
            break;
        case SYS_TELL:
            f->R.rax = tell((int)argument[0]);
            break;
        case SYS_CLOSE:
            // puts("------------------ userprog/syscall.c:hand-close             ------------------");
            close((int)argument[0]);
            break;
        //! 2/25 추가 !
        case SYS_MMAP:
            f->R.rax = mmap((void *)argument[0], (size_t)argument[1], (int)argument[2], (int)argument[3], (off_t)argument[4]);
            break;
        case SYS_MUNMAP:
            munmap((void *)argument[0]);
            break;
    }
}

void check_address(void *addr) {
    if (!is_user_vaddr(addr)) {
        exit(-1);
    }
}

void halt(void) {
    power_off();
}

void exit(int status) {
    // puts("------------------ userprog/syscall.c:exit               ------------------");
    struct thread *t = thread_current();
    t->child_exit_status = status;
    printf("%s: exit(%d)\n", t->name, status);
    thread_exit();
}

// bool create (const char *file, unsigned initial_size){
//     check_address(file);
//     if (file == NULL || strlen(file) > 14 || strlen(file) == 0){
//         return 0;
//     }
//     bool result = filesys_create(file, initial_size);
//     return result;
// }

bool create(const char *file, unsigned initial_size) {
    if (file == NULL)
        exit(-1);
    check_address(file);
    lock_acquire(&filesys_lock);
    if (strlen(file) > 14 || strlen(file) == 0) {
        return 0;
    }
    bool result = filesys_create(file, initial_size);
    lock_release(&filesys_lock);
    return result;
}

bool remove(const char *file) {
    check_address(file);
    lock_acquire(&filesys_lock);
    bool result = filesys_remove(file);
    lock_release(&filesys_lock);
    return result;
}

pid_t fork(const char *thread_name, struct intr_frame *f) {
    pid_t child_tid = process_fork(thread_name, f);
    if (child_tid >= 0) {
        sema_down(&thread_current()->fork_semaphore);
        struct thread *child_t = get_child_process(child_tid);
        // printf("--------------><----------------\n");
        // printf("--------------><----------------\n");

        if (child_t->child_fork_status == -1) {
            return -1;
        }
        return child_tid;
    } else {
        return -1;
    }
    // sema_down(&thread_current()->fork_semaphore);
    // if (thread_current()->tid == child_tid) {
    //     return 0;
    // } else {
    //     return child_tid;
    // }
}

int exec(const char *cmd_line) {
    // puts("------------------ userprog/syscall.c:exec               ------------------");
    return process_exec(cmd_line);
    // if (thread_current()->parent != NULL) {
    // }
}

int wait(pid_t pid) {
    // printf("wait current pid:%d\n", thread_current()->tid);
    // puts("------------------ userprog/syscall.c:wait               ------------------");
    return process_wait(pid);
}

int open(const char *file) {
    // puts("------------------ userprog/syscall.c:open               ------------------");
    if (file == NULL)
        return -1;

    if (thread_current()->max_fd >= 512) {
        return -1;
    }

    struct file *open_file;
    lock_acquire(&filesys_lock);
    open_file = filesys_open(file);

    if (strcmp(thread_current()->name, file) == 0) {
        file_deny_write(open_file);
    }
    lock_release(&filesys_lock);

    if (open_file == NULL) {
        return -1;
    }
    return process_add_file(open_file);
}

int filesize(int fd) {
    struct thread *curr = thread_current();
    struct file *target_file = curr->fd_table[fd];

    lock_acquire(&filesys_lock);

    if (target_file == NULL)
        return -1;

    int result = file_length(target_file);

    lock_release(&filesys_lock);
    return result;
}

int read(int fd, void *buffer, unsigned length) {
    // puts("------------------ userprog/syscall.c:read            ------------------");
    check_address(buffer);
    // if(!is_writable(spt_find_page(&thread_current()->spt, buffer))
    struct page *page = spt_find_page(&thread_current()->spt, buffer);
    if(page->writable == 0){
        exit(-1);
    }

    int result;

    if (fd == 0) {
        unsigned left_n = length;

        while (left_n) {
            left_n -= input_getc();
        }
        return length;
    } else {
        // struct file *target_file =  process_get_file(fd);

        lock_acquire(&filesys_lock);
        struct thread *curr = thread_current();
        struct file *target_file = curr->fd_table[fd];
        if (target_file == NULL)
            result = -1;

        if (target_file->deny_write) {
            result = 0;
        }

        result = file_read(target_file, buffer, length);
        lock_release(&filesys_lock);
        return result;
    }
}

int write(int fd, const void *buffer, unsigned length) {
    check_address(buffer);
    const char *new_buffer;
    new_buffer = (const char *)buffer;
    int result;

    if (fd == 1) {
        putbuf(new_buffer, length);
    } else {
        lock_acquire(&filesys_lock);
        struct file *target_file = process_get_file(fd);

        if (!target_file) {
            result = 0;
        }

        if (target_file->deny_write) {
            result = 0;
        }

        result = file_write(target_file, new_buffer, length);
        lock_release(&filesys_lock);
    }

    return result;
}

void seek(int fd, unsigned position) {
    struct file *target_file = process_get_file(fd);
        lock_acquire(&filesys_lock);
    
    file_seek(target_file, position);
        lock_release(&filesys_lock);

}

unsigned tell(int fd) {
    struct file *target_file = process_get_file(fd);
    lock_acquire(&filesys_lock);
    unsigned temp = file_tell(target_file); 
        lock_release(&filesys_lock);
    return temp;
}

void close(int fd) {
    process_close_file(fd);
}

void *mmap(void *addr, size_t length, int writable, int fd, off_t offset) {
    //tmp 내 코드
    if (fd < 2 || addr == 0) {
        return NULL;
    }
    //tmp 형우 코드
    if (fd < 2) {
        return NULL;
    }
    struct file *file = process_get_file(fd);
    if (file == NULL || file_length(file) <= 0) {
        return NULL;
    }
    if (((int)addr & (1 << 12) - 1) != 0) {  //mint page alignment를 확인해준다
        return NULL;
    }
    if (spt_find_page(&thread_current()->spt, addr)) {
        return NULL;
    }
    if (addr == NULL) {
        return NULL;
    }
    if ((long long)length <= 0) {
        return NULL;
    }
    //mint offset이 file의 길이보다 크면, NULL을 리턴한다
    if (file_length(file) <= offset) {
        return NULL;
    }
    if (offset & PGMASK) {  // PGMASK한게 존재하면! 즉, 4kb의 배수가 아니라면
        return NULL;
    }

    if (addr >= KERN_BASE) {
        return NULL;
    }

    //mint 여기서 우리가 받는 addr은 page->va 를 뜻한다.
    //mint 즉, 내가 원하는 주소에다가 mem할 수 있는 것이다. page 구조체의 va멤버가 이 값을 가지고 있다.
    check_address(addr);
    if (file_length(file) < length + offset) {
        length = file_length(file) - offset;
        if (length == 0) {
            return NULL;
        }
    }


    lock_acquire(&filesys_lock);
    struct file *newfile = file_reopen(file);
    lock_release(&filesys_lock);
    // process_add_file(newfile);

    return do_mmap(addr, length, writable, newfile, offset);
}

void munmap(void *addr){
    struct page *page = spt_find_page(&thread_current()->spt, addr);

    while(page_get_type(page) == VM_FILE){
        // printf("------------>here\n");
        do_munmap(addr);
        addr = addr + PGSIZE;
        // file_close(page->file.vafile);
        page = spt_find_page(&thread_current()->spt, addr);
        if(page == NULL){
            return;
        }
    }
}