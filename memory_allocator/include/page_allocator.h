#ifndef _PAGE_ALLOCATOR_H_
#define _PAGE_ALLOCATOR_H_

#include <sys/queue.h> // macOS
#include <bsd/sys/queue.h> // Linux
#include <stdbool.h>

#define PA_PAGE_SIZE pa_page_size()
#define PA_PAGE_MASK ~((int64_t)PA_PAGE_SIZE - 1)

typedef struct page {
    void *p_address;
    LIST_ENTRY(page) p_mazpl_link; // ma_zone page list link
    LIST_ENTRY(page) p_paht_link; // page allocator hash table link
} page_t;

LIST_HEAD(page_list, page);
typedef struct page_list page_list_t;

page_t *pa_page_alloc(void);
void pa_page_free(void *addr);

int pa_init(void);

bool pa_addr_is_page_aligned(void *addr);
bool pa_addr_is_on_page(void *addr, page_t *page);

long pa_page_size(void);
size_t pa_nalloced(void);

void pa_page_dump(page_t *p);

#endif // _PAGE_ALLOCATOR_H_
