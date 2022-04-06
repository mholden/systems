#ifndef _PAGE_ALLOCATOR_H_
#define _PAGE_ALLOCATOR_H_

#include <sys/queue.h> // macOS
#include <bsd/sys/queue.h> // Linux
#include <stdbool.h>

#define PA_PAGE_SIZE 4096
#define PA_PAGE_MASK ~(PA_PAGE_SIZE - 1)

typedef struct page {
    void *p_address;
    LIST_ENTRY(page) p_mazpl_link; // ma_zone page list link
    LIST_ENTRY(page) p_papl_link; // page allocator page list link
} page_t;

LIST_HEAD(page_list, page);
typedef struct page_list page_list_t;

page_list_t *pa_alloc(int order);
void pa_pl_release(page_list_t *pl);

page_t *pa_page_alloc(void);
void pa_page_free(page_t *page);

bool pa_addr_is_on_page(page_t *p, void *addr);

void pa_page_dump(page_t *p);

#endif // _PAGE_ALLOCATOR_H_
