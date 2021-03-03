#ifndef VM_FILE_H
#define VM_FILE_H
#include "filesys/file.h"
#include "vm/vm.h"

//! 추가 : 파일 구조체 정보 추가
typedef int32_t off_t;
typedef bool vm_initializer (struct page *, void *aux);
struct file_info{
    struct file *file;
    off_t ofs;
    size_t page_read_bytes;
    size_t page_zero_bytes;
    bool writable;
};

//! 02/25 추가
struct mmap_file{
    int mapid;
    struct file* file;
    struct list_elem elem;
    struct list page_list;
};

//! - - - --  -- - - -- - -- -- - - -- -- - -- - -- - -- -- - - - -

struct page;
enum vm_type;

struct file_page {
    vm_initializer *init;
	enum vm_type type;
    struct file_info* file_info;
    off_t offset;
    struct file *vafile; /* 가상주소와 맵핑된 파일 */
};

void vm_file_init (void);
bool file_backed_initializer (struct page *page, enum vm_type type, void *kva);
void *do_mmap(void *addr, size_t length, int writable,
		struct file *file, off_t offset);
void do_munmap (void *va);

// //! 02/25 추가
static bool
lazy_load_file(struct page *page, void *aux);
#endif
