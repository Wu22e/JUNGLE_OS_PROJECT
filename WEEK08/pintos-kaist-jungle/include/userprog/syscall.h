#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

//! 추가
#include <stdbool.h>
#include <debug.h>
#include <stddef.h>
// #include "threads/interrupt.h"
/* Process identifier. */
typedef int pid_t;
//!  - - - - - - - -
struct lock filesys_lock;

void syscall_init(void);
/* 새로 구현한 함수 */
void check_address(void* addr);
void get_argument(void* rsp, int* arg, int count);

void sys_halt(void);
void sys_exit(int status);
pid_t sys_fork(const char* thread_name);
int sys_exec(const char* file);
int sys_wait(pid_t pid);
bool sys_create(const char* file, unsigned initial_size);
bool sys_remove(const char* file);
int sys_open(const char* file);
int sys_filesize(int fd);
int sys_read(int fd, void* buffer, unsigned size);
int sys_write(int fd, void *buffer, unsigned size);
void sys_seek (int fd, unsigned position);
unsigned sys_tell (int fd);
void sys_close (int fd);

#endif /* userprog/syscall.h */

