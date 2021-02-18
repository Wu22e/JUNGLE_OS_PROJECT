#ifndef THREADS_SYNCH_H
#define THREADS_SYNCH_H

#include <list.h>
#include <stdbool.h>

/* A counting semaphore. */
struct semaphore {
	unsigned value;             /* Current value. */
	struct list waiters;        /* List of waiting threads. */
};

/* One semaphore in a list. */
struct semaphore_elem {
	struct list_elem elem;              /* List element. */
	struct semaphore semaphore;         /* This semaphore. */
};

void sema_init (struct semaphore *, unsigned value); /* semaphore�� �־��� value�� �ʱ�ȭ */
void sema_down (struct semaphore *); /* semaphore�� ��û�ϰ� ȹ������ �� value�� 1 ���� */
bool sema_try_down (struct semaphore *); 
void sema_up (struct semaphore *); /* semaphore�� ��ȯ�ϰ� value�� 1 ���� */
void sema_self_test (void);
bool cmp_sem_priority(const struct list_elem* a,
	const struct list_elem* b,
	void* aux);

/* Lock. */
struct lock {
	struct thread *holder;      /* Thread holding lock (for debugging). */ //lock�� ��û���� �������� �ּ�
	struct semaphore semaphore; /* Binary semaphore controlling access. */
};

void lock_init (struct lock *); /* lock �ڷᱸ���� �ʱ�ȭ */
void lock_acquire (struct lock *); /* lock�� ��û */
bool lock_try_acquire (struct lock *);
void lock_release (struct lock *); /* lock�� ��ȯ */
bool lock_held_by_current_thread (const struct lock *);

/* Condition variable. */
struct condition {
	struct list waiters;        /* List of waiting threads. */
};

void cond_init (struct condition *); /* condition variable �ڷᱸ���� �ʱ�ȭ */
void cond_wait (struct condition *, struct lock *); /* condition variable�� ���� signal�� ������ ��ٸ� */
void cond_signal (struct condition *, struct lock *); /* condition variable���� ��ٸ��� ���� ���� �켱������ �����忡 signal�� ���� */
void cond_broadcast (struct condition *, struct lock *); /* condition variable���� ��ٸ��� ��� �����忡 signal�� ���� */

/* Optimization barrier.
 *
 * The compiler will not reorder operations across an
 * optimization barrier.  See "Optimization Barriers" in the
 * reference guide for more information.*/
#define barrier() asm volatile ("" : : : "memory")

#endif /* threads/synch.h */
