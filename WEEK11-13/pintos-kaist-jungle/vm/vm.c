/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"


//! 추가 (ppt) 함수 선언
static unsigned vm_hash_func (const struct hash_elem *e,void *aux);
static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b);


/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
    //! 여기서 구현 시작
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}

//! 구현 (ppt에선 함수 파라미터로 void *vaddr만 받는데 어케함?;)
/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function. */
    //! 여기서부터 구현
    /* pg_round_down()으로 va의 페이지 번호를 얻음 */
    struct hash_elem *elem;
    page->va = pg_round_down(va); //? 이게 가능했던 이유가 뭐였을까?
    //! 메모리 주소고 page 주소를 얻기 위해서 
    //! 뒤에 12비트를 지웠다
    /* hash_find() 함수를 사용해서 hash_elem 구조체 얻음 */
    /* 만약 존재하지 않는다면 NULL 리턴 */
    elem = hash_find(&spt->vm, &page->elem);
    //! va는 임의의 virtual 
    /* hash_entry()로 해당 hash_elem의 vm_entry 구조체 리턴 */
	return elem != NULL ? hash_entry(elem, struct page, hash_elem) : NULL;
}

//! 구현 (ppt에선 함수이름으로 insert_vme라고 되어있음)
/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */
    //! 여기서부터 구현
    if(!hash_insert(&spt->vm, &page->elem)) succ = true;
	return succ;
}

//void //! 바꿈 : 기존 void였는데 return은 true로 되있어서 bool로 자료형 바궈줌
bool
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
    int succ = false;
    //! 여기서 부터 구현
    if(hash_delete(&spt->vm, &page->elem)) succ = true;

    //! - - - - - - - - - - - - - - - - - - - - - - - - - -
	vm_dealloc_page (page);
    return succ;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: Fill this function. */
    //! 여기서 부터 구현
    frame = palloc_get_page(PAL_USER);
    PANIC("todo");  //! 우리는 메모리가 꽉찰때 스왓아웃할 부분이 구현안되어있으니 PANIC에 걸릴거다.
    //! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */
    page = spt_find_page (&thread_current()->spt, va); //todo NULL 처리해야함
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
    //! 여기서부터 구현
    spt_insert_page(&thread_current()->spt, page) //! 얘 반환형이 bool인데, 밑에 return (swap_in)이 bool이어서 걍 
    //! 나중에 보자

	return swap_in (page, frame->kva);
}

//! 구현 시작 - - - --  -- - - -- - -- -- - - -- - -- - -
/* Initialize new supplemental page table */ 
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
    /* hash_init()으로 해시테이블 초기화 */
    hash_init(&spt->vm, vm_hash_func, vm_less_func, NULL);
    /* 인자로 해시 테이블과 vm_hash_func과 vm_less_func 사용 */
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}


//! 추가 : supplemental_page_table_init 함수에서  
void spt_destructor(struct hash_elem *e, void *aux) {
    // struct hash *h = &(&(thread_current()->spt)->vm);
    // struct hash *h = &thread_current()->spt.vm;
    // struct hash_elem *found = find_elem(h, find_bucket(h, e), e);
    // if (found != NULL) {
    //     remove_elem(h, found);
    //     rehash(h);
    // }
    // return found;
    struct page *p = hash_entry(e, struct page, elem);
    vm_dealloc_page(p);
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
    hash_destroy(&spt->vm, spt_destructor);
}


//! 추가 (ppt)
static uint64_t vm_hash_func (const struct hash_elem *e,void *aux)
{
    /* hash_entry()로 element에 대한 vm_entry 구조체 검색 */
    struct page * p = hash_entry(e, struct page, elem);
    /* hash_int()를 이용해서 page의 멤버 va에 대한 해시값을
    구하고 반환 */
    return hash_int(p->va);
}

//! 추가 (ppt)
static bool vm_less_func (const struct hash_elem *a, const struct
hash_elem *b)
{
    /* hash_entry()로 각각의 element에 대한 page 구조체를 얻은
    후 va 비교 (b가 크다면 true, a가 크다면 false */
    struct page * ap = hash_entry(a, struct page, elem);
    struct page * bp = hash_entry(b, struct page, elem);

    return ap->va < bp->va;
}