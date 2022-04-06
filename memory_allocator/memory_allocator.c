#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "memory_allocator.h"

static bool ma_initd;
static ma_zone_t ma_zones[MA_NUM_ZONES];
static lock_t ma_lock = LOCK_INITIALIZER;

static int ma_zinit(ma_zone_t *maz, size_t mcsz) {
    int err;
    
    maz->maz_mcsz = mcsz;
    err = lock_init(&maz->maz_lock);
    if (err)
        goto error_out;
    
    return 0;
    
error_out:
    return err;
}

static int ma_init(void) {
    size_t mcsz;
    int err;
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        mcsz = (1 << (i + MA_SMALLEST_ZONE_ORDER)) - sizeof(mem_chunk_header_t);
        err = ma_zinit(&ma_zones[i], mcsz);
        if (err)
            goto error_out;
    }
    
    ma_initd = true;
    
    return 0;
    
error_out:
    return err;
}

static void ma_zone_init_new_page(ma_zone_t *maz, page_t *p) {
    uint8_t *pstart;
    mem_chunk_t *mc;
    size_t mem_chunk_size;
    int mem_chunks_per_page;
    
    mem_chunk_size = maz->maz_mcsz + sizeof(mem_chunk_header_t);
    mem_chunks_per_page = PA_PAGE_SIZE / mem_chunk_size;
    
    pstart = (uint8_t *)p->p_address;
    for (int i = 0; i < mem_chunks_per_page; i++) {
        mc = (mem_chunk_t *)(pstart + (i * mem_chunk_size));
        mc->mc_header.mch_sz = 0;
        LIST_INSERT_HEAD(&maz->maz_freelist, &mc->mc_header, mch_mazfl_link);
        maz->maz_nfree++;
    }
    
    return;
}

static int ma_zone_add_page(ma_zone_t *maz) {
    page_t *p;
    int err;
    
    p = pa_page_alloc();
    if (!p) {
        err = ENOMEM;
        goto error_out;
    }
    
    LIST_INSERT_HEAD(&maz->maz_pagelist, p, p_mazpl_link);
    ma_zone_init_new_page(maz, p);
    
    maz->maz_npages++;
    
    return 0;
    
error_out:
    return err;
}

static void *ma_zalloc(ma_zone_t *maz, size_t size) {
    mem_chunk_t *mc;
    void *p;
    int err;
    
    lock_lock(&maz->maz_lock);
    
    if (LIST_EMPTY(&maz->maz_freelist)) {
        err = ma_zone_add_page(maz);
        if (err)
            goto error_out;
    }
    
    mc = (mem_chunk_t *)LIST_FIRST(&maz->maz_freelist);
    mc->mc_header.mch_sz = size;
    
    LIST_REMOVE(&mc->mc_header, mch_mazfl_link);
    maz->maz_nfree--;
    assert(maz->maz_nfree >= 0);
    
    lock_unlock(&maz->maz_lock);
    
    p = (uint8_t *)mc + sizeof(mem_chunk_header_t);
    
    return p;
    
error_out:
    lock_unlock(&maz->maz_lock);
    return NULL;
}

void *ma_alloc(size_t size) {
    void *p;
    int err;
    
    if (!ma_initd) {
        lock_lock(&ma_lock);
        if (!ma_initd) { // check again while locked
            err = ma_init();
            if (err) {
                printf("ma_alloc: ma_init() failed (%d)\n", err);
                goto error_out;
            }
        }
        lock_unlock(&ma_lock);
    }
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        if (size <= ma_zones[i].maz_mcsz) {
            p = ma_zalloc(&ma_zones[i], size);
            break;
        }
        // else pa_page_alloc()
    }
    
    return p;
    
error_out:
    return NULL;
}

static void ma_zfree(ma_zone_t *maz, mem_chunk_t *mc) {
    lock_lock(&maz->maz_lock);
    LIST_INSERT_HEAD(&maz->maz_freelist, &mc->mc_header, mch_mazfl_link);
    maz->maz_nfree++;
    lock_unlock(&maz->maz_lock);
}

void ma_free(void *p) {
    mem_chunk_header_t *mch;
    size_t size;
    
    mch = (mem_chunk_header_t *)((uint8_t *)p - sizeof(mem_chunk_header_t));
    size = mch->mch_sz;
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        if (size <= ma_zones[i].maz_mcsz) {
            ma_zfree(&ma_zones[i], (mem_chunk_t *)mch);
            break;
        }
        // else pa_page_free()
    }
    
    return;
}

static void ma_zone_teardown(ma_zone_t *maz) {
    page_t *p, *pnext;
    
    lock_lock(&maz->maz_lock);
    
    LIST_FOREACH_SAFE(p, &maz->maz_pagelist, p_mazpl_link, pnext)
        pa_page_free(p);
    
    maz->maz_npages = 0;
    LIST_INIT(&maz->maz_pagelist);
    
    maz->maz_nfree = 0;
    LIST_INIT(&maz->maz_freelist);
    
    lock_unlock(&maz->maz_lock);
    
    return;
}

void ma_teardown(void) {
    for (int i = 0; i < MA_NUM_ZONES; i++)
        ma_zone_teardown(&ma_zones[i]);
}

static size_t ma_zone_n_mem_chunks(ma_zone_t *maz) {
    size_t num_mem_chunks_per_page, num_mem_chunks;
    
    num_mem_chunks_per_page = PA_PAGE_SIZE / (maz->maz_mcsz + sizeof(mem_chunk_header_t));
    num_mem_chunks = maz->maz_npages * num_mem_chunks_per_page;
    
    return num_mem_chunks;
}

static size_t ma_zone_nalloced(ma_zone_t *maz) {
    return ma_zone_n_mem_chunks(maz) - maz->maz_nfree;
}

size_t ma_nalloced(void) {
    size_t nalloced = 0;
    
    for (int i = 0; i < MA_NUM_ZONES; i++)
        nalloced += ma_zone_nalloced(&ma_zones[i]);
    
    return nalloced;
}

static void ma_zone_dump(ma_zone_t *maz) {
    page_t *p;
    mem_chunk_header_t *mch;
    
    lock_lock(&maz->maz_lock);
    
    printf("maz_mcsz: %lu (%lu) maz_npages %lu maz_nfree %ld (n_mem_chunks %lu nalloced %lu)\n", maz->maz_mcsz,
           maz->maz_mcsz + sizeof(mem_chunk_header_t), maz->maz_npages, maz->maz_nfree, ma_zone_n_mem_chunks(maz), ma_zone_nalloced(maz));
    
    if (!LIST_EMPTY(&maz->maz_pagelist)) {
        printf("  maz_pagelist: ");
        LIST_FOREACH(p, &maz->maz_pagelist, p_mazpl_link)
            pa_page_dump(p);
        printf("\n");
    }
    
    if (!LIST_EMPTY(&maz->maz_freelist)) {
        printf("  maz_freelist: ");
        LIST_FOREACH(mch, &maz->maz_freelist, mch_mazfl_link)
            printf("%p ", mch);
        printf("\n");
    }
    
    lock_unlock(&maz->maz_lock);
}

void ma_dump(void) {
    printf("memory allocator: num zones %d (mem chunk header size %lu)\n", MA_NUM_ZONES, sizeof(mem_chunk_header_t));
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        printf("zone %d: ", i);
        ma_zone_dump(&ma_zones[i]);
        //printf("\n");
    }
}

static void ma_zone_check(ma_zone_t *maz, ma_zone_t *prev_maz) {
    page_t *p;
    mem_chunk_header_t *mch;
    uint8_t *mch_last_byte;
    size_t npages = 0, nfree = 0;
    bool is_on_page_of_pagelist;
    
    lock_lock(&maz->maz_lock);
    
    LIST_FOREACH(p, &maz->maz_pagelist, p_mazpl_link)
        npages++;
    
    assert(maz->maz_npages == npages);
    
    LIST_FOREACH(mch, &maz->maz_freelist, mch_mazfl_link) {
        assert(mch->mch_sz <= maz->maz_mcsz);
        if (prev_maz)
            assert((mch->mch_sz > prev_maz->maz_mcsz) || !mch->mch_sz);
        
        is_on_page_of_pagelist = false;
        LIST_FOREACH(p, &maz->maz_pagelist, p_mazpl_link) {
            if (pa_addr_is_on_page(p, mch)) {
                is_on_page_of_pagelist = true;
                break;
            }
        }
        assert(is_on_page_of_pagelist);
        
        mch_last_byte = (uint8_t *)mch + sizeof(mem_chunk_header_t) + maz->maz_mcsz - 1;
        assert(pa_addr_is_on_page(p, mch_last_byte));
        
        nfree++;
    }
    
    assert(maz->maz_nfree == nfree);
    
    lock_unlock(&maz->maz_lock);
}

void ma_check(void) {
    ma_zone_t *prev_zone = NULL;
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        ma_zone_check(&ma_zones[i], prev_zone);
        prev_zone = &ma_zones[i];
    }
}
