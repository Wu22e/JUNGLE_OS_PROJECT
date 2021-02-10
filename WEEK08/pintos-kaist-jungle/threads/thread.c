#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "intrinsic.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

   /* Random value for basic thread
	  Do not modify this value. */
#define THREAD_BASIC 0xd42df210

	  /* List of processes in THREAD_READY state, that is, processes
		 that are ready to run but not actually running. */
static  ready_list;

//! 추가 : THREAD_BLOCKED 상태의 스레드를 관리하기 위한 리스트 자료 구조 추가
static struct list sleep_queue;

/* Idle thread. */
static struct thread* idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread* initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Thread destruction requests */
static struct list destruction_req;

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */

//! 추가 :  sleep_list 에서 대기중인 스레드들의 wakeup_tick 값 중 최소값을 저장하기위한 변수 추가
static long long next_tick_to_awake;

/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;

static void kernel_thread(thread_func*, void* aux);

static void idle(void* aux UNUSED);
static struct thread* next_thread_to_run(void);
static void init_thread(struct thread*, const char* name, int priority);
static void do_schedule(int status);
static void schedule(void);
static tid_t allocate_tid(void);

/* Returns true if T appears to point to a valid thread. */
#define is_thread(t) ((t) != NULL && (t)->magic == THREAD_MAGIC)

/* Returns the running thread.
 * Read the CPU's stack pointer `rsp', and then round that
 * down to the start of a page.  Since `struct thread' is
 * always at the beginning of a page and the stack pointer is
 * somewhere in the middle, this locates the curent thread. */
#define running_thread() ((struct thread *) (pg_round_down (rrsp ())))


 // Global descriptor table for the thread_start.
 // Because the gdt will be setup after the thread_init, we should
 // setup temporal gdt first.
static uint64_t gdt[3] = { 0, 0x00af9a000000ffff, 0x00cf92000000ffff };

/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.
   Also initializes the run queue and the tid lock.
   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().
   It is not safe to call thread_current() until this function
   finishes. */

   //! -----------------> priority 과제를 위한 함수들 start <---------------------

   //! 추가 : list_insert_ordered() 함수에서 사용 하기 위해 정렬 방법을 결정하기 위한 함수 작성
bool
cmp_priority(const struct list_elem* a, const struct list_elem* b, void* aux UNUSED) {
	//! 첫번째 인자의 우선순위가 높으면 1을 반환, 그렇지 않으면 0을 반환
	struct thread* a_thread = list_entry(a, struct thread, elem);
	struct thread* b_thread = list_entry(b, struct thread, elem);

	if (a_thread->priority > b_thread->priority) {
		return true;
	}
	else {
		return false;
	}
}

void
test_max_priority(void)
{
	//! ready_list에서 우선순위가 가장 높은 스레드와 현재 스레드의
	//! 우선순위를 비교하여 스케줄링 한다.
	//!(ready_list 가 비어있지 않은지 확인하기)

	if (!list_empty(&ready_list)) {
		if (cmp_priority(list_begin(&ready_list), &thread_current()->elem, NULL)) {
			thread_yield();
		}
	}
}

//!---------> donate < ---------


//! 추가 :  priority donation 을 수행하는 함수를 구현한다.
void
donate_priority(void)
{
	//! 현재 스레드가 기다리고 있는 lock 과 연결 된 모든 스레드들을 순회하며
	//! 현재 스레드의 우선순위를 lock 을 보유하고 있는 스레드에게 기부 한다
	struct thread* thread = thread_current()->wait_on_lock->holder;
	while (thread != NULL) {
		if (thread->priority < thread_current()->priority) {
			thread->priority = thread_current()->priority;
		}
		if (thread->wait_on_lock != NULL) {
			thread = thread->wait_on_lock->holder;
		}
		else
			break;
	}
}

//! 추가 :  lock 을 해지 했을때 donations 리스트에서 해당 엔트리를
//!			삭제 하기 위한 함수를 구현한다.
void
remove_with_lock(struct lock* lock)
{
	struct list_elem* e = list_begin(&thread_current()->donations);

	//! 현재 스레드의 donations 리스트를 확인하여 해지 할 lock을 보유하고 있는
	//! 엔트리를 삭제한다
	while (e != list_end(&thread_current()->donations)) {
		if (list_entry(e, struct thread, donation_elem)->wait_on_lock == lock) {
			e = list_remove(e);
		}
		else
			e = list_next(e);
	}
}


//! 추가 : 스레드의 우선순위가 변경 되었을 때 donation 을 고려하여
//!			우선순위를 다시 결정 하는 함수를 작성 한다
void
refresh_priority(void)
{
	//! 현재 스레드의 우선순위를 기부 받기 전의 우선순위로 변경
	thread_current()->priority = thread_current()->init_priority;

	if (!list_empty(&thread_current()->donations)) {
		list_sort(&thread_current()->donations, cmp_priority, NULL);
		struct thread* thread = list_entry(list_begin(&thread_current()->donations), struct thread, donation_elem);
		if (thread->priority > thread_current()->priority) {
			thread_current()->priority = thread->priority;
		}
	}
}


//! -----------------> priority 과제를 위한 함수들 end <---------------------



//! -----------------> alarm 과제를 위한 함수들 start <---------------------

//! 추가 : 깨워야 할 스레드중 가장 작은 tick을 next_tick_to_awake가 갖도록 업데이트 한다 */
void
update_next_tick_to_awake(int64_t ticks) {
	if (next_tick_to_awake > ticks) {
		next_tick_to_awake = ticks;
	}
}

//! 추가 : 가장 먼저 일어나야할 스레드의 tick을 가져옴
int64_t
get_next_tick_to_awake(void) {
	return next_tick_to_awake;
}


//! 추가 : 인자로 들어온 ticks 까지 재울 생각이야.
void
thread_sleep(int64_t ticks) {
	//printf("thread_sleep4\n");
	struct thread* curr;

	enum intr_level old_level;
	old_level = intr_disable();
	//! 아래 과정 중에는 인터럽트를 받아들이지 않는다.
	//! 다만 나중에 다시 받아들이기 위해 old_level에 이전 인터럽트 상태를 담아둔다.

	curr = thread_current();
	//! 현재의 thread 주소를 가져온다.

	ASSERT(curr != idle_thread);
	//! 현재의 thread가 유휴 thread가 아니어야 한다. 

	curr->wakeup_tick = ticks;
	//printf("sleep ticks : %d", ticks);
	//! 현재의 thread의 wakeup_ticks에 인자로 들어온 ticks를 저장

	update_next_tick_to_awake(ticks);
	//! next_tick_to_awake 를 업데이트!

	list_push_back(&sleep_queue, &(curr->elem));
	//printf("\nsleep : %p\n sleep_que_end : %p\n\n", &curr->elem, list_end(&sleep_queue)->prev);
	//! sleep_queue에 현재 스레드의 elem을 푸쉬

	thread_block();
	//! 현재 thread를 block 시키자

	intr_set_level(old_level);
	//! 다시 스케줄하러 갈 수 있게 만들어두기(인터럽트 당할 수 있게)
}


//! 추가 : 인자로 들어온 현재의 ticks 보다 작거나 같은 thread를 깨울거야
void
thread_awake(int64_t ticks) {
	next_tick_to_awake = INT64_MAX; //! next_tick_to_awake 초기화
	//printf("thread_awake3 : ticks : %d\n", ticks);
	struct list_elem* curr_elem = list_begin(&sleep_queue);
	struct thread* curr_thread;
	while (curr_elem != list_end(&sleep_queue)) {
		curr_thread = list_entry(curr_elem, struct thread, elem);

		if (curr_thread->wakeup_tick <= ticks) {
			//printf("wakeup_tick : %d \n", curr_thread->wakeup_tick);
			curr_elem = list_remove(curr_elem);   //! 순서중요
			thread_unblock(curr_thread);		//! 위와 순서 바꾸면 틀림
			//printf("unblock : %d \n", curr_thread->wakeup_tick);
		}

		else {
			update_next_tick_to_awake(curr_thread->wakeup_tick);
			//printf("next sequence\n");
			curr_elem = curr_elem->next;
		}
	}

}

//! -----------------> alarm 과제를 위한 함수들 end <---------------------


void
thread_init(void) {
	ASSERT(intr_get_level() == INTR_OFF);

	/* Reload the temporal gdt for the kernel
	 * This gdt does not include the user context.
	 * The kernel will rebuild the gdt with user context, in gdt_init (). */
	struct desc_ptr gdt_ds = {
		.size = sizeof(gdt) - 1,
		.address = (uint64_t)gdt
	};
	lgdt(&gdt_ds);

	/* Init the globla thread context */
	lock_init(&tid_lock);
	list_init(&ready_list);

	//! 추가 : sleep_queue 를 초기화
	list_init(&sleep_queue);

	list_init(&destruction_req);

	/* Set up a thread structure for the running thread. */
	initial_thread = running_thread();
	init_thread(initial_thread, "main", PRI_DEFAULT);
	initial_thread->status = THREAD_RUNNING;
	initial_thread->tid = allocate_tid();
}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start(void) {
	/* Create the idle thread. */
	struct semaphore idle_started;
	sema_init(&idle_started, 0);
	thread_create("idle", PRI_MIN, idle, &idle_started);

	/* Start preemptive thread scheduling. */
	intr_enable();

	/* Wait for the idle thread to initialize idle_thread. */
	sema_down(&idle_started);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick(void) {
	struct thread* t = thread_current();

	/* Update statistics. */
	if (t == idle_thread)
		idle_ticks++;
#ifdef USERPROG
	else if (t->pml4 != NULL)
		user_ticks++;
#endif
	else
		kernel_ticks++;

	/* Enforce preemption. */
	if (++thread_ticks >= TIME_SLICE)
		intr_yield_on_return();
}

/* Prints thread statistics. */
void
thread_print_stats(void) {
	printf("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
		idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.
   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.
   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create(const char* name, int priority,
	thread_func* function, void* aux) {
	struct thread* t;
	tid_t tid;

	ASSERT(function != NULL);

	/* Allocate thread. */
	t = palloc_get_page(PAL_ZERO);   //! 페이지 할당
	if (t == NULL)
		return TID_ERROR;




	/* Initialize thread. */
	init_thread(t, name, priority);	//! thread 구조체 초기화

	//! 추가
	/* fd 값 초기화(0,1은 표준 입력,출력) */
	t->next_fd = 2;
	/* File Descriptor 테이블에 메모리 할당 */
	//! File Descriptor같은 경우 우리가 구조체에서 정적으로 선언해줬고
	//! init_thread안에서 memset으로 0으로 초기화하기때문에 굳이 메모리 할당할 필요 없음



	tid = t->tid = allocate_tid();		//! tid 할당


	t->par_tid = thread_current()->tid;
	t->process_load = 0;
	t->process_exit = 0;
	sema_init(&t->semaphore_exit, 0);
	sema_init(&t->semaphore_load, 0);
	list_push_back(&thread_current()->child, &t->child_elem);


	/* Call the kernel_thread if it scheduled.
	 * Note) rdi is 1st argument, and rsi is 2nd argument. */
	t->tf.rip = (uintptr_t)kernel_thread;
	t->tf.R.rdi = (uint64_t)function;
	t->tf.R.rsi = (uint64_t)aux;
	t->tf.ds = SEL_KDSEG;
	t->tf.es = SEL_KDSEG;
	t->tf.ss = SEL_KDSEG;
	t->tf.cs = SEL_KCSEG;
	t->tf.eflags = FLAG_IF;



		/* Add to run queue. */
		thread_unblock(t); // t는 자식

		//! 추가 : 생성된 스레드의 우선순위가 현재 실행중인 스레드의 우선순위보다 높다면 CPU를 양보한다.
	if (t->priority > thread_current()->priority) {
		thread_yield();
	}

	return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().
   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block(void) {
	ASSERT(!intr_context());
	ASSERT(intr_get_level() == INTR_OFF);
	thread_current()->status = THREAD_BLOCKED;
	schedule();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)
   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock(struct thread* t) {
	enum intr_level old_level;

	ASSERT(is_thread(t));

	old_level = intr_disable(); //! 인터럽트 배리어
	ASSERT(t->status == THREAD_BLOCKED);

	struct list_elem* curr_elem = list_begin(&ready_list);
	struct thread* curr_thread;

	//! 추가 : 우선순위 순으로 정렬 되어 ready_list에 삽입되도록 하자
	list_insert_ordered(&ready_list, &t->elem, cmp_priority, NULL);

	// list_push_back (&ready_list, &t->elem);  --> 기본코드

	t->status = THREAD_READY;
	intr_set_level(old_level); //! 인터럽트 배리어 풀기
}

/* Returns the name of the running thread. */
const char*
thread_name(void) {
	return thread_current()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread*
	thread_current(void) {   // 현재 실행되고 있는 thread를 반환
	struct thread* t = running_thread();

	/* Make sure T is really a thread.
	   If either of these assertions fire, then your thread may
	   have overflowed its stack.  Each thread has less than 4 kB
	   of stack, so a few big automatic arrays or moderate
	   recursion can cause stack overflow. */
	ASSERT(is_thread(t));
	ASSERT(t->status == THREAD_RUNNING);

	return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid(void) {
	return thread_current()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit(void) {
	ASSERT(!intr_context());

#ifdef USERPROG
	process_exit();
#endif

	/* Just set our status to dying and schedule another process.
	   We will be destroyed during the call to schedule_tail(). */
	intr_disable();
	do_schedule(THREAD_DYING);
	NOT_REACHED();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield(void) {			//! CPU를 양보하고, thread를 ready_list에 삽입
	struct thread* curr = thread_current();
	enum intr_level old_level;

	ASSERT(!intr_context());

	old_level = intr_disable();
	if (curr != idle_thread)
		//! 추가 : ready_list에 삽입 될 때 우선순위 순서로 정렬
		list_insert_ordered(&ready_list, &thread_current()->elem, cmp_priority, NULL);
	//list_push_back (&ready_list, &curr->elem);  --> 기존 코드
	do_schedule(THREAD_READY);
	intr_set_level(old_level);
}

/* Sets the current thread's priority to NEW_PRIORITY. */
//! thread의 priority 를 다시 세팅한다. 이는 사용자가 사용하는 함수라 생각해야 할 것 같다. 
void
thread_set_priority(int new_priority)
{
	thread_current()->priority = new_priority;

	//! 추가 : thread의 우선순위를 아예 변경하는 것이므로 init_priority도 변경
	thread_current()->init_priority = new_priority;

	//! 추가 : refresh_priority() 함수를 사용하여 우선순위를 변경으로 인한
	//!			donation 관련 정보를 갱신한다
	refresh_priority();

	//! 추가 : donation_priority(), test_max_pariority() 함수를 적절히
	//! 		사용하여 priority donation 을 수행하고 스케줄링 한다
	test_max_priority();


	// 추가 : thread의 우선순위가 변경되었을 때 우선 순위에 따라 선점이 발생하도록 한다.
	// if (cmp_priority(list_begin(&ready_list), &thread_current()->elem, NULL)){
	// 	thread_yield();
	// }      ---> 기존 변경코드
}

/* Returns the current thread's priority. */
int
thread_get_priority(void) {
	return thread_current()->priority;
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice(int nice UNUSED) {
	/* TODO: Your implementation goes here */
}

/* Returns the current thread's nice value. */
int
thread_get_nice(void) {
	/* TODO: Your implementation goes here */
	return 0;
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg(void) {
	/* TODO: Your implementation goes here */
	return 0;
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu(void) {
	/* TODO: Your implementation goes here */
	return 0;
}

/* Idle thread.  Executes when no other thread is ready to run.
   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle(void* idle_started_ UNUSED) {
	struct semaphore* idle_started = idle_started_;

	idle_thread = thread_current();
	sema_up(idle_started);

	for (;;) {
		/* Let someone else run. */
		intr_disable();
		thread_block();

		/* Re-enable interrupts and wait for the next one.
		   The `sti' instruction disables interrupts until the
		   completion of the next instruction, so these two
		   instructions are executed atomically.  This atomicity is
		   important; otherwise, an interrupt could be handled
		   between re-enabling interrupts and waiting for the next
		   one to occur, wasting as much as one clock tick worth of
		   time.
		   See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
		   7.11.1 "HLT Instruction". */
		asm volatile ("sti; hlt" : : : "memory");
	}
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread(thread_func* function, void* aux) {
	ASSERT(function != NULL);

	intr_enable();       /* The scheduler runs with interrupts off. */
	function(aux);       /* Execute the thread function. */
	thread_exit();       /* If function() returns, kill the thread. */
}


/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread(struct thread* t, const char* name, int priority) {
	ASSERT(t != NULL);
	ASSERT(PRI_MIN <= priority && priority <= PRI_MAX);
	ASSERT(name != NULL);

	memset(t, 0, sizeof * t);
	t->status = THREAD_BLOCKED;
	strlcpy(t->name, name, sizeof t->name);
	t->tf.rsp = (uint64_t)t + PGSIZE - sizeof(void*);
	t->priority = priority;
	t->magic = THREAD_MAGIC;

	//! 추가 : priority donation 관련 자료구조 초기화
	t->init_priority = priority;  //! thread의 priority를 사용자가 직접 
								  //! 바꾸는게 아닌 이상 바뀔일 없음
	t->wait_on_lock = NULL;
	list_init(&t->donations);
	list_init(&t->child);
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread*
next_thread_to_run(void) {
	if (list_empty(&ready_list))
		return idle_thread;
	else
		return list_entry(list_pop_front(&ready_list), struct thread, elem);
}

/* Use iretq to launch the thread */
void
do_iret(struct intr_frame* tf) {
	__asm __volatile(
	"movq %0, %%rsp\n"
		"movq 0(%%rsp),%%r15\n"
		"movq 8(%%rsp),%%r14\n"
		"movq 16(%%rsp),%%r13\n"
		"movq 24(%%rsp),%%r12\n"
		"movq 32(%%rsp),%%r11\n"
		"movq 40(%%rsp),%%r10\n"
		"movq 48(%%rsp),%%r9\n"
		"movq 56(%%rsp),%%r8\n"
		"movq 64(%%rsp),%%rsi\n"
		"movq 72(%%rsp),%%rdi\n"
		"movq 80(%%rsp),%%rbp\n"
		"movq 88(%%rsp),%%rdx\n"
		"movq 96(%%rsp),%%rcx\n"
		"movq 104(%%rsp),%%rbx\n"
		"movq 112(%%rsp),%%rax\n"
		"addq $120,%%rsp\n"
		"movw 8(%%rsp),%%ds\n"
		"movw (%%rsp),%%es\n"
		"addq $32, %%rsp\n"
		"iretq"
		: : "g" ((uint64_t)tf) : "memory");
}

/* Switching the thread by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.
   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.
   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function. */
static void
thread_launch(struct thread* th) {
	uint64_t tf_cur = (uint64_t)&running_thread()->tf;
	uint64_t tf = (uint64_t)&th->tf;
	ASSERT(intr_get_level() == INTR_OFF);

	/* The main switching logic.
	 * We first restore the whole execution context into the intr_frame
	 * and then switching to the next thread by calling do_iret.
	 * Note that, we SHOULD NOT use any stack from here
	 * until switching is done. */
	__asm __volatile(
	/* Store registers that will be used. */
	"push %%rax\n"
		"push %%rbx\n"
		"push %%rcx\n"
		/* Fetch input once */
		"movq %0, %%rax\n"
		"movq %1, %%rcx\n"
		"movq %%r15, 0(%%rax)\n"
		"movq %%r14, 8(%%rax)\n"
		"movq %%r13, 16(%%rax)\n"
		"movq %%r12, 24(%%rax)\n"
		"movq %%r11, 32(%%rax)\n"
		"movq %%r10, 40(%%rax)\n"
		"movq %%r9, 48(%%rax)\n"
		"movq %%r8, 56(%%rax)\n"
		"movq %%rsi, 64(%%rax)\n"
		"movq %%rdi, 72(%%rax)\n"
		"movq %%rbp, 80(%%rax)\n"
		"movq %%rdx, 88(%%rax)\n"
		"pop %%rbx\n"              // Saved rcx
		"movq %%rbx, 96(%%rax)\n"
		"pop %%rbx\n"              // Saved rbx
		"movq %%rbx, 104(%%rax)\n"
		"pop %%rbx\n"              // Saved rax
		"movq %%rbx, 112(%%rax)\n"
		"addq $120, %%rax\n"
		"movw %%es, (%%rax)\n"
		"movw %%ds, 8(%%rax)\n"
		"addq $32, %%rax\n"
		"call __next\n"         // read the current rip.
		"__next:\n"
		"pop %%rbx\n"
		"addq $(out_iret -  __next), %%rbx\n"
		"movq %%rbx, 0(%%rax)\n" // rip
		"movw %%cs, 8(%%rax)\n"  // cs
		"pushfq\n"
		"popq %%rbx\n"
		"mov %%rbx, 16(%%rax)\n" // eflags
		"mov %%rsp, 24(%%rax)\n" // rsp
		"movw %%ss, 32(%%rax)\n"
		"mov %%rcx, %%rdi\n"
		"call do_iret\n"
		"out_iret:\n"
		: : "g"(tf_cur), "g" (tf) : "memory"
		);
}

/* Schedules a new process. At entry, interrupts must be off.
 * This function modify current thread's status to status and then
 * finds another thread to run and switches to it.
 * It's not safe to call printf() in the schedule(). */
static void
do_schedule(int status) {
	ASSERT(intr_get_level() == INTR_OFF);
	ASSERT(thread_current()->status == THREAD_RUNNING); //! 러닝중인 스레드가 아니어야함
	while (!list_empty(&destruction_req)) {
		struct thread* victim =
			list_entry(list_pop_front(&destruction_req), struct thread, elem);
		palloc_free_page(victim);
	}
	thread_current()->status = status;
	schedule();
}

static void
schedule(void) {
	struct thread* curr = running_thread();
	struct thread* next = next_thread_to_run();

	ASSERT(intr_get_level() == INTR_OFF);
	ASSERT(curr->status != THREAD_RUNNING); //! 러닝중인 스레드여야 함
	ASSERT(is_thread(next));
	/* Mark us as running. */
	next->status = THREAD_RUNNING;

	/* Start new time slice. */
	thread_ticks = 0;

#ifdef USERPROG
	/* Activate the new address space. */
	process_activate(next);
#endif

	if (curr != next) {
		/* If the thread we switched from is dying, destroy its struct
		   thread. This must happen late so that thread_exit() doesn't
		   pull out the rug under itself.
		   We just queuing the page free reqeust here because the page is
		   currently used bye the stack.
		   The real destruction logic will be called at the beginning of the
		   schedule(). */
		if (curr && curr->status == THREAD_DYING && curr != initial_thread) {
			ASSERT(curr != next);
			list_push_back(&destruction_req, &curr->elem);
		}

		/* Before switching the thread, we first save the information
		 * of current running. */
		thread_launch(next);
	}
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid(void) {
	static tid_t next_tid = 1;
	tid_t tid;

	lock_acquire(&tid_lock);
	tid = next_tid++;
	lock_release(&tid_lock);

	return tid;
}