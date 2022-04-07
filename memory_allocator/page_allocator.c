#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "page_allocator.h"
#include "synch.h"

// TODO: implement an actual page allocator with the ability to allocate and free more than one page

static long _pa_page_size;
static uint8_t _pa_page_bits;
static ssize_t _pa_nalloced;
static lock_t pa_lock = LOCK_INITIALIZER;

#define PA_HASHTABLE_SIZE 1024
page_list_t *pa_hashtable; // so that we can find a page_t given a page address in pa_page_free

int pa_init(void) {
    int err;
    
    _pa_page_size = sysconf(_SC_PAGE_SIZE);
    if (_pa_page_size < 0) {
        err = errno;
        goto error_out;
    }
    
    // page size should be at least 1024 and should be aligned
    assert((_pa_page_size >= (1 << 10)) && ((_pa_page_size & ((1 << 10) - 1)) == 0));
    
    while (((_pa_page_size >> _pa_page_bits) & 1) == 0)
        _pa_page_bits++;
    
    //printf("pa_init: _pa_page_size: %lu _pa_page_bits: %" PRIu8 " PA_PAGE_MASK 0x%llx\n", _pa_page_size, _pa_page_bits, PA_PAGE_MASK);
    
    pa_hashtable = malloc(sizeof(page_list_t) * PA_HASHTABLE_SIZE);
    if (!pa_hashtable) {
        err = ENOMEM;
        goto error_out;
    }
    
    memset(pa_hashtable, 0, sizeof(page_list_t) * PA_HASHTABLE_SIZE);
    
    return 0;
    
error_out:
    return err;
}

bool pa_addr_is_page_aligned(void *addr) {
    return (((uintptr_t)addr & (uintptr_t)PA_PAGE_MASK) == (uintptr_t)addr);
}

bool pa_addr_is_on_page(void *addr, page_t *page) {
    return (((uintptr_t)addr & (uintptr_t)PA_PAGE_MASK) == (uintptr_t)page->p_address);
}

size_t pa_page_number(void *addr) {
    return (uintptr_t)addr >> (uintptr_t)_pa_page_bits;
}

page_t *pa_page_alloc(void) {
    page_t *p;
    page_list_t *pl;
    size_t htind;
    
    p = malloc(sizeof(page_t));
    if (!p)
        goto error_out;
    
    memset(p, 0, sizeof(page_t));
    
    p->p_address = mmap(NULL, _pa_page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p->p_address == MAP_FAILED) {
        printf("pa_page_alloc: mmap failed (errno %d)\n", errno);
        goto error_out;
    }
    
    assert(pa_addr_is_page_aligned(p->p_address));
    
    htind = pa_page_number(p->p_address) % PA_HASHTABLE_SIZE;
    pl = &pa_hashtable[htind];
    
    lock_lock(&pa_lock);
    
    LIST_INSERT_HEAD(pl, p, p_paht_link);
    _pa_nalloced++;
    
    lock_unlock(&pa_lock);
    
    return p;
    
error_out:
    if (p)
        free(p);
    
    return NULL;
}

void pa_page_free(void *addr) {
    page_t *p;
    page_list_t *pl;
    size_t htind;
    int err;
    
    htind = pa_page_number(addr) % PA_HASHTABLE_SIZE;
    pl = &pa_hashtable[htind];
    
    lock_lock(&pa_lock);
    
    LIST_FOREACH(p, pl, p_paht_link) {
        if (p->p_address == addr)
            break;
    }
    
    if (!p) {
        printf("pa_page_free: addr %p not in pa_hashtable\n", addr);
        goto out;
    }
    
    err = munmap(p->p_address, _pa_page_size);
    if (err)
        printf("pa_page_free: munmap failed on %p\n", addr);
    
    LIST_REMOVE(p, p_paht_link);
    free(p);
    
    _pa_nalloced--;
    assert(_pa_nalloced >= 0);
    
out:
    lock_unlock(&pa_lock);
        
    return;
}

long pa_page_size(void) {
    return _pa_page_size;
}

size_t pa_nalloced(void) {
    return _pa_nalloced;
}

void pa_page_dump(page_t *p) {
    printf("page @ %p: p_address %p ", p, p->p_address);
}
