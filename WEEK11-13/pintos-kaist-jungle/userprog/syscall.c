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
    uint64_t argument[3] = {f->R.rdi, f->R.rsi, f->R.rdx};

    struct intr_frame *cur_if = &thread_current()->tf;

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
            // puts("------------------ userprog/syscall.c:handler:read    ------------------");
            f->R.rax = read((int)argument[0], (void *)argument[1], (unsigned)argument[2]);
            break;
        case SYS_WRITE:
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
    if (strlen(file) > 14 || strlen(file) == 0) {
        return 0;
    }
    bool result = filesys_create(file, initial_size);
    return result;
}

bool remove(const char *file) {
    check_address(file);
    bool result = filesys_remove(file);
    return result;
}

pid_t fork(const char *thread_name, struct intr_frame *f) {
    pid_t child_tid = process_fork(thread_name, f);
    if (child_tid >= 0) {
        struct thread *child_t = get_child_process(child_tid);

        sema_down(&thread_current()->fork_semaphore);
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
    process_exec(cmd_line);
    if (thread_current()->parent != NULL) {
    }
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
    open_file = filesys_open(file);

    if (strcmp(thread_current()->name, file) == 0) {
        file_deny_write(open_file);
    }

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

    int result;

    if (fd == 0) {
        unsigned left_n = length;

        while (left_n) {
            left_n -= input_getc();
        }
        return length;

    } else {
        lock_acquire(&filesys_lock);
        // struct file *target_file =  process_get_file(fd);

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
    file_seek(target_file, position);
}

unsigned tell(int fd) {
    struct file *target_file = process_get_file(fd);
    return file_tell(target_file);
}

void close(int fd) {
    process_close_file(fd);
}