#ifndef VM_ANON_H
#define VM_ANON_H
#include "vm/vm.h"
struct page;
enum vm_type;
typedef bool vm_initializer (struct page *, void *aux);
struct anon_page {
    //! 2/27 추가
    vm_initializer *init;
	enum vm_type type;
	void *aux;
    uint32_t sector; //! bitmap의 bit_cnt
};

void vm_anon_init (void);
bool anon_initializer (struct page *page, enum vm_type type, void *kva);

#endif
