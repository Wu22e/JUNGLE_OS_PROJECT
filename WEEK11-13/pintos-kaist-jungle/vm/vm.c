/* vm.c: Generic interface for virtual memory objects. */

#include "vm/vm.h" 

#include "threads/malloc.h"
#include "vm/inspect.h"

//! 헤더 추가(pml4_set_page(mmu) 사용하기 위함)
#include "threads/mmu.h"

//! 추가 (ppt) 함수 선언
static unsigned vm_hash_func(const struct hash_elem *e, void *aux);
static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b);

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void vm_init(void) {
    list_init(&victim_list);  //! 2/26 추가 
    vm_anon_init();
    vm_file_init();
#ifdef EFILESYS /* For project 4 */
    pagecache_init();
#endif
    register_inspect_intr();
    /* DO NOT MODIFY UPPER LINES. */
    /* TODO: Your code goes here. */
    //! 여기서 구현 시작
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type(struct page *page) {
    int ty = VM_TYPE(page->operations->type);
    switch (ty) {
        case VM_UNINIT:
            return VM_TYPE(page->uninit.type);
        default:
            return ty;
    }
}

/* Helpers */
static struct frame *vm_get_victim(void);
static bool vm_do_claim_page(struct page *page);
static struct frame *vm_evict_frame(void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
//_________________________________________________
/* Create an uninitialized page with the given type. 
The swap_in handler of uninit page automatically initializes the page according to the type,
and calls INIT with given AUX. Once you have the page struct, insert the page into the process's supplementary page table. 
Using VM_TYPE macro defined in vm.h can be handy. */
//_________________________________________________
//^~~~~~~~~~~~~~~~~~ON GITBOOK
//: 커널이 새로운 페이지 요청이 들어오면 이것이 Invoke됨
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
                                    vm_initializer *init, void *aux) {
    //^~~~~~~~~~ uninit.h에 있음
    ASSERT(VM_TYPE(type) != VM_UNINIT);
    // upage = pg_round_down(upage);
    struct supplemental_page_table *spt = &thread_current()->spt;
    struct page *page = spt_find_page(spt, upage);
    /* Check wheter the upage is already occupied or not. */
    if (page == NULL) {
        /* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
        //todo 마지막 you should 부분이 이해가안감
        //!  여기서 부터 구현 시작
        page = (struct page *)malloc(sizeof(struct page));
        // printf("*------------   page :: %p\n", page);
        ASSERT(page);
        switch (VM_TYPE(type)) {
            case VM_ANON:
                uninit_new(page, upage, init, type, aux, anon_initializer);
                break;
            case VM_FILE:
                uninit_new(page, upage, init, type, aux, file_backed_initializer);
                break;
            default:
            // printf("here\n");
                PANIC("in vm_alloc_page");
            // printf("there\n");
        }
        page->thread = thread_current(); //! 추가 : 2/26 현재 스레드를 page 구조체안에 저장 
        page->writable = writable;  //todo 이거 확인해야함
        /* TODO: Insert the page into the spt. */
        return spt_insert_page(spt, page);
    }
err:
    ASSERT("err in vm_alloc_page_with_initializer");  //! 추가 : 실패시 palloc한거 해제해야함
    return false;
}

//! 구현 (ppt에선 함수 파라미터로 void *vaddr만 받는데 어케함?;)
/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
    struct page page;
    struct hash_elem *e;
    page.va = pg_round_down(va);
    e = hash_find(&spt->vm, &page.elem);
    return e != NULL ? hash_entry(e, struct page, elem) : NULL;
}

//! 구현 (ppt에선 함수이름으로 insert_vme라고 되어있음)
/* Insert PAGE into spt with validation. */
bool spt_insert_page(struct supplemental_page_table *spt UNUSED,
                     struct page *page UNUSED) {
    int succ = false;
    /* TODO: Fill this function. */
    //! 여기서부터 구현
    if (!hash_insert(&spt->vm, &page->elem)) {
        succ = true;
    }
    return succ;
}

//void //! 바꿈 : 기존 void였는데 return은 true로 되있어서 bool로 자료형 바궈줌
bool spt_remove_page(struct supplemental_page_table *spt, struct page *page) {
    int succ = false;
    //! 여기서 부터 구현
    if (hash_delete(&spt->vm, &page->elem)) succ = true;

    //! - - - - - - - - - - - - - - - - - - - - - - - - - -
    // vm_dealloc_page(page);
    return succ;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim(void) {
    struct frame *victim = NULL;
    /* TODO: The policy for eviction is up to you. */
    //! 2/27 여기서 부터 구현
    struct list_elem *victim_elem = list_pop_front(&victim_list);
    struct page *victim_page = list_entry(victim_elem, struct page, victim);
    victim = victim_page->frame;
    return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame(void) {
    struct frame *victim UNUSED = vm_get_victim();
    /* TODO: swap out the victim and return the evicted frame. */
    //! 2/27 여기서 부터 구현
    bool succ = false;
    succ = swap_out(victim->page);
    //! 우선 이 함수는 vm_get_frame에서 palloc했는데, 물리메모리가 다찼을 경우
    //! victim을 정해준다.
    //! 이때 여기서 free 하고 palloc 안해주는 이유는 
    //! vm_get_victim에서 얻어준 frame 주소는 어짜피
    //! free하고 할당해도 그 주소일 거니까 이함수가 호출끝나고
    //!  vm_get_frame으로 가서 바로 할당해준다.
    if(succ){
        return victim;
    }
    else{
        return NULL;
    }
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
//^ ------------->vm_get_frame 함수<-------------------------------
//^ 함수 역할 :유저풀로부터 새로운 물리페이지를 할당받는다.
//^ 함수 설명 : palloc(PAL_USER)를 통해 USER PAGE POOL에서 페이지를 받아온다.
//^ (이때 palloc 함수가 리턴하는 주소값은 커널영역에 해당하는 va이다.)
//^ 그래서 frame->kva 에 palloc을 해줌
static struct frame *
vm_get_frame(void) {
    struct frame *frame = NULL;
    /* TODO: Fill this function. */
    //! 여기서 부터 구현
    frame = (struct frame *)malloc(sizeof(struct frame));
    //? 여기서 malloc을 쓰면 kernel pool을 쓰게 된다.
    //? malloc 함수에 들어가면 size가 1kb 이상이면 그냥 페이지 크기를 할당받는다
    //? 그게 아니면 2의 몇승짜리로 페이지하나의 영역의 쪼개들어가서 찾아서 들어간다.
    //! 왜 malloc을 하는거지?;
    //! 나름 생각해보면 여기서 frame은 "실제 디스크"를 생각했을때는 엄청 많은 data가들어가지만
    //! struct frame은 그냥 metadata기 때문이고 포인터로 선언되어있기떄문에
    //! palloc해서 굳이 4kb나 할당해 줄필요없이 malloc으로 sizeof(struct frame*)만큼의
    //! 메모리만 할당해 주면 된다.
    frame->page = NULL;  //todo 그냥 이렇게 초기화하는게 맞는가?
    frame->kva = palloc_get_page(PAL_USER); 

    //! 2/27 추가
    if(frame->kva == NULL){//! palloc 실패시 evict frame 함수 호출
        struct frame *evict_frame = vm_evict_frame(); 
        frame->kva = evict_frame->kva;
    }

    ASSERT(frame != NULL);
    ASSERT(frame->page == NULL);  //! 여기서 frame->page == NULL일때 통과하는데 왜지?
    return frame;
}

/* Growing the stack. */
static void
vm_stack_growth(void *addr UNUSED) {
    ASSERT(spt_find_page(&thread_current()->spt, addr) == NULL);
    void *new_stack_bottom = pg_round_down(addr);
    vm_alloc_page(VM_ANON | VM_MARKER_0, new_stack_bottom, true);
    vm_claim_page(new_stack_bottom);
    struct page *page = spt_find_page(&thread_current()->spt, new_stack_bottom);
    // page->anon.sector = -1;
    new_stack_bottom = new_stack_bottom + PGSIZE;
    page = spt_find_page(&thread_current()->spt, new_stack_bottom);
    while (page == NULL) {
        vm_alloc_page(VM_ANON | VM_MARKER_0, new_stack_bottom, true);
        // page->anon.sector = -1;
        vm_claim_page(new_stack_bottom);
        new_stack_bottom = new_stack_bottom + 4096;
        page = spt_find_page(&thread_current()->spt, new_stack_bottom);
    }
    // memset(new_stack_bottom, 0, PGSIZE); //mint 얘는 넣어주면 안될거 같기도 하다..
}
/* Handle the fault on write_protected page */
static bool
vm_handle_wp(struct page *page UNUSED) {
}

/* Return true on success */
bool vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED,
                         bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
    struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
    struct page *page = NULL;
    /* TODO: Validate the fault */
    /* TODO: Your code goes here */
    if(is_kernel_vaddr(addr) || addr == NULL){
            // printf("1\n");

        return false;
    }

    void* rsp_stack = is_kernel_vaddr(f->rsp) ? thread_current()->rsp_stack : f->rsp;

    page = spt_find_page(spt, addr);

    if(not_present){
        if(page == NULL){
            if(rsp_stack - addr <= 8 && USER_STACK - 0x100000 <= addr && addr <= USER_STACK){
                vm_stack_growth(addr);
                // printf("sibala1\n");
            // printf("2\n");

                return true;
            }
            // printf("addr is %p\n" , addr);
            // printf("rsp-stack is %p\n", rsp_stack);
            // printf("3\n");
            return false;
        }
        else{
            // if(page->writable == 0 && write){
            //     return false;
            // }
            // printf("sibala2\n");
            // printf("4\n");

            return vm_do_claim_page(page);
        }
    }
            // printf("5\n");

    return false;

    // return false;


    // if (page == NULL) {
    //     if ((uintptr_t)f->rsp - (uintptr_t)addr <= 8){
    //             vm_stack_growth(addr);
    //     }
    //     else{
    //     // printf("---------->i m 2\n");

    //         exit(-1);
    //     }
    // }
    // else{
    //     if(page->writable == 0 && write){
    //     // printf("---------->i m 3\n");

    //         exit(-1);
    //     }
    //     return vm_do_claim_page(page);
    // }
}




// bool vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED, bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
//     struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
//     struct page *page = NULL;
//     /* TODO: Validate the fault */
//     /* TODO: Your code goes here */
//     page = spt_find_page(spt, addr);
//     if (page == NULL) {
//        /*
//        (1) PF가 발생했고, 
//        (2) 해당 Faulting Address가 SPT에 등록되어있지 않은 영역 (아직 할당되지 않은 페이지)인 경우,
//         Fatulting Address을 확인해서 Faulting Address가 (rsp - 8)이상이고 USER_STACK 미만인 경우에 
//         한해서 stack을 추가 할당해주시면 됩니다.
//        */
//         if (((int)addr- (int)f->rsp) >= -8 && user && write && not_present && (addr < USER_STACK)) { //!3.3 
//             if (pg_round_down(addr) <= USER_STACK - (1 << 20)) { //mint 등호 추가해줌
//                 return;
//             }
//             vm_stack_growth(addr);
//         } else {
//             // puts("wrong address access");
//             exit(-1);
//         }
//     } else {
//         if (page->writable == 0 && write == 1) {
//             // puts("try to write to a wrong place");
//             exit(-1);
//         }
//         return vm_do_claim_page(page);
//     }
// }




/* Free the page. 
 * DO NOT MODIFY THIS FUNCTION. */
void vm_dealloc_page(struct page *page) {
    destroy(page);
    free(page);
}

//^ ------------->vm_claim_page 함수<-------------------------------
//^ 함수 역할 : 인자 va(user virtual address임)를 받아서 거기에 해당하는
//^ 		 	psge를 찾아서 넣어줌(이때 이 page는 유저영역의 page임)
/* Claim the page that allocate on VA. */
bool vm_claim_page(void *va UNUSED) {
    struct page *page = NULL;
    /* TODO: Fill this function */
    if (page = spt_find_page(&thread_current()->spt, va)) {
        return vm_do_claim_page(page);
    }  //todo NULL 처리해야함
    else
        return false;
}

//^ ------------->vm_do_claim_page 함수<-------------------------------
//^ 함수 역할 :
//^ 함수 설명 : vm_get_frame을 통해 얻은 frame과 page를 page table을 통해 연결해줌
/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page(struct page *page) {
    struct frame *frame = vm_get_frame();

    /* Set links */
    frame->page = page;
    page->frame = frame;
    /* TODO: Insert page table entry to map page's VA to frame's PA. */
    //! 여기서부터 구현
    // spt_insert_page(&thread_current()->spt, page); //! 얘 반환형이 bool인데, 밑에 return (swap_in)이 bool이어서 걍
    //
    //pml4_for_each?
    if (!pml4_set_page(thread_current()->pml4, page->va, frame->kva, page->writable))
        ASSERT("fail to pml4_set_page");
    
    
    // page->thread = thread_current();  //! 2/26 성현 피셜



    //! 여기서 page->va는 user virtual address임
    //! pml4_set_page에서 page walk가 성공하면 true, 실패하면 false 반환
    //! 이 함수에서 physical address로 연결해줌
    // spt_insert_page(&page->thread->spt, page);
    list_push_back(&victim_list, &page->victim); //! 여기서 victim list에 넣어준다.
    
    return swap_in(page, frame->kva);  //todo 나중에 이해
}

//! 구현 시작 - - - --  -- - - -- - -- -- - - -- - -- - -
/* Initialize new supplemental page table */
void supplemental_page_table_init(struct supplemental_page_table *spt UNUSED) {
    /* hash_init()으로 해시테이블 초기화 */
    hash_init(&spt->vm, vm_hash_func, vm_less_func, NULL);
    /* 인자로 해시 테이블과 vm_hash_func과 vm_less_func 사용 */
}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED,
                                  struct supplemental_page_table *src UNUSED) {
    // Copies the supplemental page table from src to dst.
    // Iterate through each page in the src's supplemental page table and make a exact copy of the entry in the dst's supplemental page table.
    // You will need to allocate uninit page and claim them immediately.
    /* 
    sky 이 함수가 실행되는 맥락은 child 일 때이다.
    sky 방법은 모르겠지만, 부모의 src를 받아서 나의 dst로 정보를 옮기면 된다. 
    */
    struct hash_iterator i;
    hash_first(&i, &src->vm);
    bool succ = false;
    while (hash_next(&i)) {
        struct page *page = hash_entry(hash_cur(&i), struct page, elem);
        enum vm_type type = page->operations->type;
        if (type == VM_UNINIT) {
            struct file_info *file_info = (struct file_info *)malloc(sizeof(struct file_info));
            if (file_info == NULL) {
                PANIC("spt_copy malloc failed");
            }
            memcpy(file_info, page->uninit.aux, sizeof(struct file_info));
            succ = vm_alloc_page_with_initializer(page->uninit.type, page->va, page->writable, page->uninit.init, file_info);
        } else {
            //mint vm_alloc_with_initializer 에서 끝에 인자 두개를 null, null로 한 것
            // 1. 페이지 새로 만들기. vm_alloc_page에서 spt_insert_page를 해준다
            // 2. 만든 페이지를 찾기
            // 3. vm_claim_page를 통해 3.1 frame만들기 3.2 frame 만든거 연결해주기
            // 4. 만들어진 frame->kva에 부모의 frame->kva memcpy해주기
            succ = vm_alloc_page(type, page->va, page->writable);
            struct page *child_page = spt_find_page(dst, page->va);
            succ = vm_claim_page(page->va);
            memcpy(child_page->frame->kva, page->frame->kva, PGSIZE);
        }
    }
    return succ;
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
    //
    struct page *p = hash_entry(e, struct page, elem);
    vm_dealloc_page(p);
}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED) {
    /* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
    struct hash_iterator i;
    if(!hash_empty(&spt->vm)){
        hash_first(&i, &spt->vm);

        while (hash_next(&i)) {
            struct page *page = hash_entry(hash_cur(&i), struct page, elem);
            destroy(page);
        }
    }
}

//! 추가 (ppt)
static unsigned vm_hash_func(const struct hash_elem *e, void *aux) {
    /* hash_entry()로 element에 대한 vm_entry 구조체 검색 */
    struct page *p = hash_entry(e, struct page, elem);
    /* hash_int()를 이용해서 page의 멤버 va에 대한 해시값을
    구하고 반환 */
    return hash_bytes(&p->va, sizeof(p->va));
}

//! 추가 (ppt)
static bool vm_less_func(const struct hash_elem *a, const struct
                         hash_elem *b) {
    /* hash_entry()로 각각의 element에 대한 page 구조체를 얻은
    후 va 비교 (b가 크다면 true, a가 크다면 false */
    struct page *ap = hash_entry(a, struct page, elem);
    struct page *bp = hash_entry(b, struct page, elem);

    return ap->va < bp->va;
}