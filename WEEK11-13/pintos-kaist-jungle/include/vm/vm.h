#ifndef VM_VM_H
#define VM_VM_H
#include <stdbool.h>

#include "threads/palloc.h"

//! 추가 (헤더 추가)
#include "hash.h"  //! 추가 : 커널 라이브러리에 있으면 경로 없어도 된다.
#include "threads/vaddr.h"
// //! 3/3 추가 : file_reopen에 filesys_lock 걸어주기 위함
// #include "userprog/syscall.h"
// #include "threads/thread.h" //! 2/26 victim list 관리를 위해 추가
#define DEBUGx

struct list victim_list; //! 2/26 추가 : victim을 저장할 list (전역으로 관리)
//! thread안에 넣으면 임의의 thread a가 ram을 다 쓸경우 다른 thread b가 swap out하고싶어도
//! thread a의 page가 소유하고있으므로 할수없음

enum vm_type {
    /* page not initialized */
    VM_UNINIT = 0,
    /* page not related to the file, aka anonymous page */
    VM_ANON = 1,
    /* page that realated to the file */
    VM_FILE = 2,
    /* page that hold the page cache, for project 4 */
    VM_PAGE_CACHE = 3,

    /* Bit flags to store state */

    /* Auxillary bit flag marker for store information. You can add more
	 * markers, until the value is fit in the int. */
    VM_MARKER_0 = (1 << 3),
    VM_MARKER_1 = (1 << 4),

    /* DO NOT EXCEED THIS VALUE. */
    VM_MARKER_END = (1 << 31),
};

#include "vm/anon.h"
#include "vm/file.h"
#include "vm/uninit.h"
#ifdef EFILESYS
#include "filesys/page_cache.h"
#endif

struct page_operations;
struct thread;

#define VM_TYPE(type) ((type)&7)

/* The representation of "page".
 * This is kind of "parent class", which has four "child class"es, which are
 * uninit_page, file_page, anon_page, and page cache (project4).
 * DO NOT REMOVE/MODIFY PREDEFINED MEMBER OF THIS STRUCTURE. */
struct page {
    const struct page_operations *operations;       //! 뭘까, 깃북에 있음
    void *va; /* Address in terms of user space */  //! 얘가 vaddr인듯
    struct frame *frame;                            /* Back reference for frame */

    /* Your implementation */
    //! 추가 - - - - - - - - - - - - - - - - - - - - - - - -
    // uint8_t type; /* VM_BIN, VM_FILE, VM_ANON의 타입 */

    // void *vaddr; /* vm_entry가 관리하는 가상페이지 번호 */

    /* Per-type data are binded into the union.
	 * Each function automatically detects the current union */
    //! 얘가 type인듯
    union {
        struct uninit_page uninit;
        struct anon_page anon;
        struct file_page file;
#ifdef EFILESYS
        struct page_cache page_cache;
#endif
    };

    bool writable; /* True일 경우 해당 주소에 write 가능
    False일 경우 해당 주소에 write 불가능 */

    bool is_loaded; /* 물리메모리의 탑재 여부를 알려주는 플래그 */

    

    /* Memory Mapped File 에서 다룰 예정 */
    // struct list_elem mmap_elem; /* mmap 리스트 element */

    // size_t offset; /* 읽어야 할 파일 오프셋 */

    // size_t read_bytes; /* 가상페이지에 쓰여져 있는 데이터 크기 */

    // size_t zero_bytes; /* 0으로 채울 남은 페이지의 바이트 */

    /* Swapping 과제에서 다룰 예정 */
    size_t swap_slot; /* 스왑 슬롯 */

    /* ‘vm_entry들을 위한 자료구조’ 부분에서 다룰 예정 */
    struct hash_elem elem; /* 해시 테이블 Element */

    struct thread* thread;
    struct list_elem victim; //! 2/27 희생자 선정하기위해 선언
                           //! 추가 - - - - - - - - - - - - - - - - - - - - - - - -
};

/* The representation of "frame" */
struct frame {
    void *kva;
    struct page *page;
};

/* The function table for page operations.
 * This is one way of implementing "interface" in C.
 * Put the table of "method" into the struct's member, and
 * call it whenever you needed. */
struct page_operations {
    bool (*swap_in)(struct page *, void *);
    bool (*swap_out)(struct page *);
    void (*destroy)(struct page *);
    enum vm_type type;
};

#define swap_in(page, v) (page)->operations->swap_in((page), v)
#define swap_out(page) (page)->operations->swap_out(page)
#define destroy(page) \
    if ((page)->operations->destroy) (page)->operations->destroy(page)

/* Representation of current process's memory space.
 * We don't want to force you to obey any specific design for this struct.
 * All designs up to you for this. */
struct supplemental_page_table {
    struct hash vm;
};

#include "threads/thread.h"
void supplemental_page_table_init(struct supplemental_page_table *spt);
bool supplemental_page_table_copy(struct supplemental_page_table *dst,
                                  struct supplemental_page_table *src);
void supplemental_page_table_kill(struct supplemental_page_table *spt);
struct page *spt_find_page(struct supplemental_page_table *spt,
                           void *va);
bool spt_insert_page(struct supplemental_page_table *spt, struct page *page);
bool spt_remove_page(struct supplemental_page_table *spt, struct page *page);

void vm_init(void);
bool vm_try_handle_fault(struct intr_frame *f, void *addr, bool user,
                         bool write, bool not_present);

#define vm_alloc_page(type, upage, writable) \
    vm_alloc_page_with_initializer((type), (upage), (writable), NULL, NULL)
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage,
                                    bool writable, vm_initializer *init, void *aux);
void vm_dealloc_page(struct page *page);
bool vm_claim_page(void *va);
enum vm_type page_get_type(struct page *page);

#endif /* VM_VM_H */
