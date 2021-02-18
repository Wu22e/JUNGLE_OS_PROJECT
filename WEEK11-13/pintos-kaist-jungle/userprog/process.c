#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/mmu.h"
#include "threads/vaddr.h"
#include "intrinsic.h"
#ifdef VM
#include "vm/vm.h"
#endif
#define FORKx
#define WAIT


static void process_cleanup (void);
static bool load (const char *file_name, struct intr_frame *if_);
static void initd (void *f_name);
static void __do_fork (void *);

//! 유저 스택에 파싱된 토큰을 저장하는 함수
void* argument_stack(char** parse, int count, void** esp);

//! 추가 : 실행 중인 파일의 데이터 변경을 예방하기 위한 전역 lock 구조체 만듦
struct lock file_exec_lock;


/* General process initializer for initd and other process. */
static void
process_init (void) {
	struct thread *current = thread_current ();
}

/* Starts the first userland program, called "initd", loaded from FILE_NAME.
 * The new thread may be scheduled (and may even exit)
 * before process_create_initd() returns. Returns the initd's
 * thread id, or TID_ERROR if the thread cannot be created.
 * Notice that THIS SHOULD BE CALLED ONCE. */
tid_t
process_create_initd (const char *file_name) { //! 유저프로그램 실행을 위한 준비단계
	char *fn_copy;
	tid_t tid;

	/* Make a copy of FILE_NAME.
	 * Otherwise there's a race between the caller and load(). */
	fn_copy = palloc_get_page (0);
	if (fn_copy == NULL)
		return TID_ERROR;
	strlcpy (fn_copy, file_name, PGSIZE);

	//! 추가 : 첫번째 인자 파싱
	char* tmp;
	file_name = strtok_r(file_name, " ", &tmp);
	
	lock_init(&file_exec_lock); //! 추가 file_exec_lock의 초기화를 process_create_initd에서 해줘야 겟다고 생각함 (아닐수도)
	
	/* Create a new thread to execute FILE_NAME. */
	tid = thread_create (file_name, PRI_DEFAULT, initd, fn_copy);
	if (tid == TID_ERROR)
		palloc_free_page (fn_copy);
	return tid;
}

/* A thread function that launches first user process. */
static void
initd (void *f_name) {  // ! 유저 프로그램 실행
#ifdef VM
	supplemental_page_table_init (&thread_current ()->spt);
#endif

	process_init ();

	if (process_exec (f_name) < 0)
		PANIC("Fail to launch initd\n");
	NOT_REACHED ();
}

/* Clones the current process as `name`. Returns the new process's thread id, or
 * TID_ERROR if the thread cannot be created. */
tid_t
process_fork (const char *name, struct intr_frame *if_ UNUSED) {
	/* Clone current thread to new thread.*/
    tid_t temp = thread_create (name, PRI_DEFAULT, __do_fork, thread_current ());
    if (temp != TID_ERROR)
    {
        sema_down(&thread_current()->semaphore_fork);
        if(thread_current()->success_fork){
            return temp;
        }
        else { 
            return -1;
        }
    }
    else {
        return TID_ERROR;
    }
}

#ifndef VM
/* Duplicate the parent's address space by passing this function to the
 * pml4_for_each. This is only for the project 2. */
static bool
duplicate_pte (uint64_t *pte, void *va, void *aux) {
	struct thread *current = thread_current ();
	struct thread *parent = (struct thread *) aux;
	void *parent_page;
	void *newpage;
	bool writable;

	/* 1. TODO: If the parent_page is kernel page, then return immediately. */
    //! 추가 : parent_page가 커널 영역이냐??
    if(!is_user_vaddr(va)) return true; //! 왜 true여야 할까?
    //! 커널이면, 기본적으로 이미 pte 복사가 되있다. 
    //! 그래서 밑에 복사하는 과정이 필요가 없다. (어쩃든 복제된 건 맞으니까 true)

	/* 2. Resolve VA from the parent's page map level 4. */
	parent_page = pml4_get_page (parent->pml4, va);


	/* 3. TODO: Allocate new PAL_USER page for the child and set result to
	 *    TODO: NEWPAGE. */
    //! 추가 : 자식도 메모리 할당 받아야지? 근데 유저영역에 받을건뎅
    newpage = palloc_get_page(PAL_USER);

	/* 4. TODO: Duplicate parent's page to the new page and
	 *    TODO: check whether parent's page is writable or not (set WRITABLE
	 *    TODO: according to the result). */

    memcpy(newpage, parent_page, PGSIZE);

    //! 추가 : 인자로 받은 pte가 writable 하냐??
    writable = is_writable(pte);

	/* 5. Add new page to child's page table at address VA with WRITABLE
	 *    permission. */
	if (!pml4_set_page (current->pml4, va, newpage, writable)) {
		/* 6. TODO: if fail to insert page, do error handling. */
        //! 에러 핸들링! 이게 맞을까?
        // palloc_free_page(newpage); //! 추가: pml4를 set page하지 못했을때 free 해주고 나가야 함._
        //! 다시 주석처리함. 이거 주석해도 잘되는거 보니 다른데서 free를 해주나봄
        //! 어디서 free하는지는 찾아봐야할듯;;
        //todo 어디서 free해줄까?
        return false;
	}
	return true;
}
#endif

/* A thread function that copies parent's execution context.
 * Hint) parent->tf does not hold the userland context of the process.
 *       That is, you are required to pass second argument of process_fork to
 *       this function. */
static void
__do_fork (void *aux) {
	struct intr_frame if_;
	struct thread *parent = (struct thread *) aux;
	struct thread *current = thread_current ();
	/* TODO: somehow pass the parent_if. (i.e. process_fork()'s if_) */
	struct intr_frame *parent_if;
	bool succ = true;

	//! parent_if 에 부모 intr_frame을 가져와야 하는데 parent->tf일까 ?? ㄴㄴ
    parent_if = &parent->fork_tf;

    /* 1. Read the cpu context to local stack. */
	memcpy (&if_, parent_if, sizeof (struct intr_frame));

	/* 2. Duplicate PT */
	current->pml4 = pml4_create();
	if (current->pml4 == NULL)
		goto error;

	process_activate (current);
#ifdef VM
	supplemental_page_table_init (&current->spt);
	if (!supplemental_page_table_copy (&current->spt, &parent->spt))
		goto error;
#else
	if (!pml4_for_each (parent->pml4, duplicate_pte, parent))
		goto error;
#endif

	/* TODO: Your code goes here.
	 * TODO: Hint) To duplicate the file object, use `file_duplicate`
	 * TODO:       in include/filesys/file.h. Note that parent should not return
	 * TODO:       from the fork() until this function successfully duplicates
	 * TODO:       the resources of parent.*/

    parent->success_fork = 1;

    //! 추가 : 부모꺼 복사해와!!! 뭐를?? 파일들을!!
    for(int i = parent->next_fd; i > 0; i--){
        if(parent->fd_table[i]!= NULL ){
            current->fd_table[i] = file_duplicate(parent->fd_table[i]);
        }
    }

    current->next_fd = parent->next_fd;
    //! 추가 : 자식이 하는동안 부모는 딱 기다려!
    sema_up(&parent->semaphore_fork);

	process_init ();

	/* Finally, switch to the newly created process. */
	if (succ)
        if_.R.rax = 0;  //! 자식은 0 리턴
		do_iret (&if_);
error:
    sema_up(&parent->semaphore_fork); //! 추가 : ERROR시에도 세마업해줘야함!!! 
	thread_exit ();
}

/* Switch the current execution context to the f_name.
 * Returns -1 on fail. */
int
process_exec(void* f_name) {
	// char* file_name = f_name; //! 잠깐 제거
    
    char* file_name = palloc_get_page(0); //! palloc_get_page에 인자안넘겨줘서 kernel에서 할당
    memcpy(file_name, f_name, strlen(f_name)+1);
	
    bool success;

	/* We cannot use the intr_frame in the thread structure.
	 * This is because when current thread rescheduled,
	 * it stores the execution information to the member. */
	struct intr_frame _if;
	_if.ds = _if.es = _if.ss = SEL_UDSEG;
	_if.cs = SEL_UCSEG;
	_if.eflags = FLAG_IF | FLAG_MBS;

    // printf("------d----> %s <-----------\n",new_file_name);

	/* We first kill the current context */
	process_cleanup();
    // memcpy(file_name, new_file_name, sizeof f_name);
    // printf("------d----> %s <-----------\n",new_file_name);

	//! 토큰화해서 앞 인자를 load의 첫번째 인자로 넣는다.
	char* tmp;
	file_name = strtok_r(file_name, " ", &tmp);
	/* And then load the binary */
	success = load(file_name, &_if);

	/* If load failed, quit. */

	if (!success){
#ifdef FORK
		thread_current()->is_process_load = 0; //! 이건 ppt 따라하다가 ...
#endif
		return -1;
	}

#ifdef FORK
	thread_current()->is_process_load = 1; //! 이건 ppt 따라하다가 ...
#endif
	//! 인자들을 스택에 먼저 쌓는다.
#ifdef FORK
	sema_up(&thread_current()->semaphore_load); //! 이건 ppt 따라하다가 ...
#endif

	char* parse[100];
	int i = 0;
	parse[i] = file_name;

	while (parse[i]) {
		i++;
		parse[i] = strtok_r(NULL, " ", &tmp);
	}

	//! 추가:
	_if.R.rsi = argument_stack(parse, i, &(_if.rsp));
	_if.R.rdi = i;
	// hex_dump(_if.rsp, _if.rsp, USER_STACK - _if.rsp, true);
	//! USER_STACK 공부잘하기
	//! - - - - - -
    
	palloc_free_page(file_name);
    if(is_kernel_vaddr(f_name)){
        palloc_free_page(f_name);
    }
    //! 얘 넣어으면 다터짐
    //! 근데 f_name은 process_create_initd에서 palloc했는데
    //! 에러가 발생하지않아서 free되지않은상태로 쭊내려와서
    //! 여기서 프리할려고했는데 터짐 그이유는 모르겟음 나중에 
    //! 보긴봐야하지만 잘모르겠음 
    //todo 주의깊게 봐야함

	/* Start switched process. */
	do_iret(&_if);
	NOT_REACHED();
}


//! 추가
void*
argument_stack(char** parse, int argc, void** rsp) {
	ASSERT(argc >= 0);

	int i, len = 0;
	void* addr[argc];

	for (i = 0; i < argc; i++) {
		len = strlen(parse[i]) + 1;
		*rsp -= len;
		memcpy(*rsp, parse[i], len);
		addr[i] = *rsp;
	}

	//! word align
	*rsp = (void*)((unsigned int)(*rsp) & 0xfffffff8);


	//! last null
	*rsp -= 8;
	*((uint64_t*)*rsp) = 0;

	//! setting **rsp with argvs
	for (i = argc - 1; i >= 0; i--) {
		*rsp -= 8;
		*((void**)*rsp) = addr[i];
	}
	//! setting fake
	*rsp -= 8;
	*((int*)*rsp) = 0;

	return ((void *)(*rsp + 8));
	// //! setting **argv (addr of stack, rsp)
	// *rsp -= 8;
	// *((void**)*rsp) = (*rsp + 8);

	// //! setting argc
	// *rsp -= 8;
	// *((int*)*rsp) = argc;

	
}

/* Waits for thread TID to die and returns its exit status.  If
 * it was terminated by the kernel (i.e. killed due to an
 * exception), returns -1.  If TID is invalid or if it was not a
 * child of the calling process, or if process_wait() has already
 * been successfully called for the given TID, returns -1
 * immediately, without waiting.
 *
 * This function will be implemented in problem 2-2.  For now, it
 * does nothing. */

int
process_wait(tid_t child_tid UNUSED) {
	/* XXX: Hint) The pintos exit if process_wait (initd), we recommend you
	 * XXX:       to add infinite loop here before
	 * XXX:       implementing the process_wait. */
#ifndef WAIT 
	while (1) {
	}
	return -1;
#endif

#ifdef WAIT
	struct thread* child_thread = get_child_process(child_tid); //!  자식 프로세스의 프로세스 디스크립터 검색
	
    if (child_thread == NULL) {
		return -1; //! 예외 처리 발생시 -1 리턴
	}

	sema_down(&child_thread->semaphore_exit); //! 자식프로세스가 종료될 때까지 부모 프로세스 대기(세마포어 이용)
	
	int temp;

	if (child_thread->is_process_exit == 0)
		temp = -1;
	else
		temp = child_thread->exit_status;

	remove_child_process(child_thread); //!  자식 프로세스 디스크립터 삭제
	return temp; //!  자식 프로세스의 exit status 리턴
#endif
}

/* Exit the process. This function is called by thread_exit (). */
void
process_exit(void) {
	struct thread* curr = thread_current();
	uint32_t* pd;

	/* TODO: Your code goes here.
	 * TODO: Implement process termination message (see
	 * TODO: project2/process_termination.html).
	 * TODO: We recommend you to implement process resource cleanup here. */
	
	// printf("%s: exit(%d)\n", curr->name, curr->exit_status);


	/* 파일 디스크립터 테이블의 최대값을 이용해 파일 디스크립터
	   의 최소값인 2가 될 때까지 파일을 닫음 */
	   /* 파일 디스크립터 테이블 메모리 해제 */

	/* 프로세스에 열린 모든 파일을 닫음 */
	for (int i = 2; i < curr->next_fd; i++){
		if(curr->fd_table[i] != NULL){
			process_close_file(i);
		}
		else {}
	}

    palloc_free_page(curr->fd_table); //! 추가 : 투포인터로 선언한 fd table palloc한걸 
    //! process_exit에서 free해준다 (왜냐면 무조건 어떤 방식으로든 프로그램 종료시 여기를 거치기 때문)
    

	process_cleanup();
	//! load 마지막 (goto에서 done으로 간 부분)에서 file_close한걸 이함수 끝부분에
	//! cleanup 밑에다가 추가해봄
	file_close(thread_current()->file_exec); //! 추가 : rox 때문

	sema_up(&curr->semaphore_exit); //! 부모프로세스의 대기 상태 이탈(세마포어 이용)
}

/* Free the current process's resources. */
static void
process_cleanup (void) {
	struct thread *curr = thread_current ();

#ifdef VM
	supplemental_page_table_kill (&curr->spt);
#endif

	uint64_t *pml4;
	/* Destroy the current process's page directory and switch back
	 * to the kernel-only page directory. */
	pml4 = curr->pml4;
	if (pml4 != NULL) {
		/* Correct ordering here is crucial.  We must set
		 * cur->pagedir to NULL before switching page directories,
		 * so that a timer interrupt can't switch back to the
		 * process page directory.  We must activate the base page
		 * directory before destroying the process's page
		 * directory, or our active page directory will be one
		 * that's been freed (and cleared). */
		curr->pml4 = NULL;
		pml4_activate (NULL);
		pml4_destroy (pml4);
	}
}

/* Sets up the CPU for running user code in the nest thread.
 * This function is called on every context switch. */
void
process_activate (struct thread *next) {
	/* Activate thread's page tables. */
	pml4_activate (next->pml4);

	/* Set thread's kernel stack for use in processing interrupts. */
	tss_update (next);
}

//! 추가한 함수
int process_add_file(struct file* f)
{
	/* 파일 객체를 파일 디스크립터 테이블에 추가
	/* 파일 디스크립터의 최대값 1 증가 */
	/* 파일 디스크립터 리턴 */
	if (f == NULL) return -1;

	struct thread* curr = thread_current();

    //! 추가 : oom 을 위해 추가
    if(curr->next_fd >= 126)
    {
        file_close(f);
        return -1;
    }

	int fd = curr->next_fd;
	curr->fd_table[fd] = f;
	curr->next_fd++;
	
	return fd;
}

//! 추가한 함수
struct file* process_get_file(int fd)
{
	/* 파일 디스크립터에 해당하는 파일 객체를 리턴 */
	/* 없을 시 NULL 리턴 */
	struct thread* curr = thread_current();
	// struct file* f = curr->fd_table[fd];
	// if(curr->fd_table[fd]) return curr->fd_table[fd];

	//! fd<=1 을 뺴준이유는 stdin stdout이 제대로작동하는데도
	//! 여기서 걸러지면 문제가생길까봐 일단 주석처리해둠
	if (fd <= 1 || fd >= curr->next_fd) return NULL;
	// if (fd >= curr->next_fd) return NULL;
	else return curr->fd_table[fd];
}

//! 추가한 함수
void process_close_file(int fd)
{
	/* 파일 디스크립터에 해당하는 파일을 닫음 */
	/* 파일 디스크립터 테이블 해당 엔트리 초기화 */
	struct thread* curr = thread_current();
	//! fd<=1 을 뺴준이유는 stdin stdout이 제대로작동하는데도
	//! 여기서 걸러지면 문제가생길까봐 일단 주석처리해둠
	if (fd <= 1 || fd >= curr->next_fd) return;
	// if (fd >= curr->next_fd) return;

	file_close(curr->fd_table[fd]); //! oom 추가: 왜 얘는 주석 처리되있었지?
	curr->fd_table[fd] = NULL;
}



//! 추가
struct thread* get_child_process(int pid)
{

	/* 자식 리스트에 접근하여 프로세스 디스크립터 검색 */
	/* 해당 pid가 존재하면 프로세스 디스크립터 반환 */
	/* 리스트에 존재하지 않으면 NULL 리턴 */

	struct list_elem* e = list_begin(&thread_current()->child);
	struct thread* e_thread;
	while (e != list_end(&thread_current()->child)) {
		e_thread = list_entry(e, struct thread, child_elem);
		if (e_thread->tid == pid) {
			return e_thread;
		}
		e = list_next(e);
	}
	return NULL;
}

//! 추가
void remove_child_process(struct thread* cp) {
	list_remove(&cp->child_elem);
	palloc_free_page((void *)cp);
}

/* We load ELF binaries.  The following definitions are taken
 * from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
#define EI_NIDENT 16

#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
 * This appears at the very beginning of an ELF binary. */
struct ELF64_hdr {
	unsigned char e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

struct ELF64_PHDR {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};

/* Abbreviations */
#define ELF ELF64_hdr
#define Phdr ELF64_PHDR

static bool setup_stack (struct intr_frame *if_);
static bool validate_segment (const struct Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes,
		bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
 * Stores the executable's entry point into *RIP
 * and its initial stack pointer into *RSP.
 * Returns true if successful, false otherwise. */
static bool
load (const char *file_name, struct intr_frame *if_) {
	struct thread *t = thread_current ();
	struct ELF ehdr;
	struct file *file = NULL;
	off_t file_ofs;
	bool success = false;
	int i;
	/* Allocate and activate page directory. */
	t->pml4 = pml4_create ();
	if (t->pml4 == NULL)
		goto done;
	
	process_activate (thread_current ());

	//! 추가 : 락 획득
	lock_acquire(&file_exec_lock);
	/* Open executable file. */
	file = filesys_open (file_name);
	if (file == NULL) {
		lock_release(&file_exec_lock);
		printf ("load: %s: open failed\n", file_name);
		goto done;
	}


	//! 추가 :  thread 구조체의 run_file을 현재 실행할 파일로 초기화 후
	//! file_deny_write()를 이용하여 파일에 대한 write를 거부 후
	//! 락 해제
	thread_current()->file_exec = file; 
	file_deny_write(thread_current()->file_exec);
	lock_release(&file_exec_lock);
	//! - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - -- - - - - - -  


	/* Read and verify executable header. */
	if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
			|| memcmp (ehdr.e_ident, "\177ELF\2\1\1", 7)
			|| ehdr.e_type != 2
			|| ehdr.e_machine != 0x3E // amd64
			|| ehdr.e_version != 1
			|| ehdr.e_phentsize != sizeof (struct Phdr)
			|| ehdr.e_phnum > 1024) {
		printf ("load: %s: error loading executable\n", file_name);
		goto done;
	}



	/* Read program headers. */
	file_ofs = ehdr.e_phoff;
	for (i = 0; i < ehdr.e_phnum; i++) {
		struct Phdr phdr;

		if (file_ofs < 0 || file_ofs > file_length (file))

			goto done;
		file_seek (file, file_ofs);

		if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)

			goto done;
		file_ofs += sizeof phdr;
		switch (phdr.p_type) {
			case PT_NULL:
			case PT_NOTE:
			case PT_PHDR:
			case PT_STACK:
			default:
				/* Ignore this segment. */
				break;
			case PT_DYNAMIC:
			case PT_INTERP:
			case PT_SHLIB:

				goto done;
			case PT_LOAD:
				if (validate_segment (&phdr, file)) {
					bool writable = (phdr.p_flags & PF_W) != 0;
					uint64_t file_page = phdr.p_offset & ~PGMASK;
					uint64_t mem_page = phdr.p_vaddr & ~PGMASK;
					uint64_t page_offset = phdr.p_vaddr & PGMASK;
					uint32_t read_bytes, zero_bytes;
					if (phdr.p_filesz > 0) {
						/* Normal segment.
						 * Read initial part from disk and zero the rest. */
						read_bytes = page_offset + phdr.p_filesz;
						zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
								- read_bytes);
					} else {
						/* Entirely zero.
						 * Don't read anything from disk. */
						read_bytes = 0;
						zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
					}
					if (!load_segment (file, file_page, (void *) mem_page,
								read_bytes, zero_bytes, writable))
						goto done;
				}
				else

					goto done;
				break;
		}
	}

	/* Set up stack. */
	if (!setup_stack (if_))

		goto done;

	/* Start address. */
	if_->rip = ehdr.e_entry;

	/* TODO: Your code goes here.
	 * TODO: Implement argument passing (see project2/argument_passing.html). */

	success = true;

done:
	/* We arrive here whether the load is successful or not. */
	// file_close (file); //! 잠시 주석, process_exit에서 닫을거임
    //! 여기서 닫으면 done 됬을때만 닫기 때문에 process_exit에서 닫으면
    //! 에러 났을때도 닫을 수 있다. (rox 문제 해결위함)
	return success;
}


/* Checks whether PHDR describes a valid, loadable segment in
 * FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Phdr *phdr, struct file *file) {
	/* p_offset and p_vaddr must have the same page offset. */
	if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK))
		return false;

	/* p_offset must point within FILE. */
	if (phdr->p_offset > (uint64_t) file_length (file))
		return false;

	/* p_memsz must be at least as big as p_filesz. */
	if (phdr->p_memsz < phdr->p_filesz)
		return false;

	/* The segment must not be empty. */
	if (phdr->p_memsz == 0)
		return false;

	/* The virtual memory region must both start and end within the
	   user address space range. */
	if (!is_user_vaddr ((void *) phdr->p_vaddr))
		return false;
	if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
		return false;

	/* The region cannot "wrap around" across the kernel virtual
	   address space. */
	if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
		return false;

	/* Disallow mapping page 0.
	   Not only is it a bad idea to map page 0, but if we allowed
	   it then user code that passed a null pointer to system calls
	   could quite likely panic the kernel by way of null pointer
	   assertions in memcpy(), etc. */
	if (phdr->p_vaddr < PGSIZE)
		return false;

	/* It's okay. */
	return true;
}

#ifndef VM
/* Codes of this block will be ONLY USED DURING project 2.
 * If you want to implement the function for whole project 2, implement it
 * outside of #ifndef macro. */

/* load() helpers. */
static bool install_page (void *upage, void *kpage, bool writable);

/* Loads a segment starting at offset OFS in FILE at address
 * UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
 * memory are initialized, as follows:
 *
 * - READ_BYTES bytes at UPAGE must be read from FILE
 * starting at offset OFS.
 *
 * - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.
 *
 * The pages initialized by this function must be writable by the
 * user process if WRITABLE is true, read-only otherwise.
 *
 * Return true if successful, false if a memory allocation error
 * or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes, bool writable) {
	ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
	ASSERT (pg_ofs (upage) == 0);
	ASSERT (ofs % PGSIZE == 0);

	file_seek (file, ofs);
	while (read_bytes > 0 || zero_bytes > 0) {
		/* Do calculate how to fill this page.
		 * We will read PAGE_READ_BYTES bytes from FILE
		 * and zero the final PAGE_ZERO_BYTES bytes. */
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		/* Get a page of memory. */
		uint8_t *kpage = palloc_get_page (PAL_USER);
		if (kpage == NULL)
			return false;

		/* Load this page. */
		if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes) {
			palloc_free_page (kpage);
			return false;
		}
		memset (kpage + page_read_bytes, 0, page_zero_bytes);

		/* Add the page to the process's address space. */
		if (!install_page (upage, kpage, writable)) {
			printf("fail\n");
			palloc_free_page (kpage);
			return false;
		}

		/* Advance. */
		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		upage += PGSIZE;
	}
	return true;
}

/* Create a minimal stack by mapping a zeroed page at the USER_STACK */
static bool
setup_stack (struct intr_frame *if_) {
	uint8_t *kpage;
	bool success = false;

	kpage = palloc_get_page (PAL_USER | PAL_ZERO);
	if (kpage != NULL) {
		success = install_page (((uint8_t *) USER_STACK) - PGSIZE, kpage, true);
		if (success)
			if_->rsp = USER_STACK;
		else
			palloc_free_page (kpage);
	}
	return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
 * virtual address KPAGE to the page table.
 * If WRITABLE is true, the user process may modify the page;
 * otherwise, it is read-only.
 * UPAGE must not already be mapped.
 * KPAGE should probably be a page obtained from the user pool
 * with palloc_get_page().
 * Returns true on success, false if UPAGE is already mapped or
 * if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable) {
	struct thread *t = thread_current ();

	/* Verify that there's not already a page at that virtual
	 * address, then map our page there. */
	return (pml4_get_page (t->pml4, upage) == NULL
			&& pml4_set_page (t->pml4, upage, kpage, writable));
}
#else
/* From here, codes will be used after project 3.
 * If you want to implement the function for only project 2, implement it on the
 * upper block. */

static bool
lazy_load_segment (struct page *page, void *aux) {
	/* TODO: Load the segment from the file */
	/* TODO: This called when the first page fault occurs on address VA. */
	/* TODO: VA is available when calling this function. */
}

/* Loads a segment starting at offset OFS in FILE at address
 * UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
 * memory are initialized, as follows:
 *
 * - READ_BYTES bytes at UPAGE must be read from FILE
 * starting at offset OFS.
 *
 * - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.
 *
 * The pages initialized by this function must be writable by the
 * user process if WRITABLE is true, read-only otherwise.
 *
 * Return true if successful, false if a memory allocation error
 * or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes, bool writable) {
	ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
	ASSERT (pg_ofs (upage) == 0);
	ASSERT (ofs % PGSIZE == 0);

	while (read_bytes > 0 || zero_bytes > 0) {
		/* Do calculate how to fill this page.
		 * We will read PAGE_READ_BYTES bytes from FILE
		 * and zero the final PAGE_ZERO_BYTES bytes. */
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		/* TODO: Set up aux to pass information to the lazy_load_segment. */
		void *aux = NULL;
		if (!vm_alloc_page_with_initializer (VM_ANON, upage,
					writable, lazy_load_segment, aux))
			return false;

		/* Advance. */
		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		upage += PGSIZE;
	}
	return true;
}

/* Create a PAGE of stack at the USER_STACK. Return true on success. */
static bool
setup_stack (struct intr_frame *if_) {
	bool success = false;
	void *stack_bottom = (void *) (((uint8_t *) USER_STACK) - PGSIZE);

	/* TODO: Map the stack on stack_bottom and claim the page immediately.
	 * TODO: If success, set the rsp accordingly.
	 * TODO: You should mark the page is stack. */
	/* TODO: Your code goes here */

	return success;
}
#endif /* VM */
