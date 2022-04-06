#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "page_allocator.h"

// TODO: implement an actual page allocator
page_list_t *pa_alloc(int order) {
    page_list_t *pl;
    page_t *p, *pnext, *plast;
    void *pages_chunk;
    int npages;
    
    pl = malloc(sizeof(page_list_t));
    if (!pl)
        goto error_out;
    
    memset(pl, 0, sizeof(page_list_t));
    
    npages = (1 << order);
    
    pages_chunk = malloc(npages * PA_PAGE_SIZE);
    if (!pages_chunk)
        goto error_out;
    
    for (int i = 0; i < npages; i++) {
        p = malloc(sizeof(page_t));
        if (!p)
            goto error_out;
        
        memset(p, 0, sizeof(page_t));
        p->p_address = (uint8_t *)pages_chunk + (i * PA_PAGE_SIZE);
        
        if (LIST_EMPTY(pl))
            LIST_INSERT_HEAD(pl, p, p_papl_link);
        else
            LIST_INSERT_AFTER(plast, p, p_papl_link);
        
        plast = p;
    }
    
    return pl;
    
error_out:
    if (pages_chunk)
        free(pages_chunk);
    if (pl) {
        LIST_FOREACH_SAFE(p, pl, p_papl_link, pnext)
            free(p);
        free(pl);
    }
    
    return NULL;
}

void pa_pl_release(page_list_t *pl) {
    free(pl);
}

page_t *pa_page_alloc(void) {
    page_list_t *pl;
    page_t *p;
    
    pl = pa_alloc(0);
    if (!pl)
        goto error_out;
    
    p = LIST_FIRST(pl);
    pa_pl_release(pl);
    
    return p;
    
error_out:
    return NULL;
}

void pa_page_free(page_t *p) {
    free(p->p_address);
    free(p);
}

bool pa_addr_is_on_page(page_t *p, void *addr) {
    return (addr >= p->p_address) && (addr < (p->p_address + PA_PAGE_SIZE));
}

void pa_page_dump(page_t *p) {
    printf("page @ %p: p_address %p ", p, p->p_address);
}
