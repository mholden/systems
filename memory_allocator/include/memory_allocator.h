#ifndef _MEMORY_ALLOCATOR_H_
#define _MEMORY_ALLOCATOR_H_

#include <stddef.h>
#include <sys/queue.h> // macOS
#include <bsd/sys/queue.h> // Linux

#include "page_allocator.h"
#include "synch.h"

#define MA_SMALLEST_ZONE_ORDER 6
#define MA_NUM_ZONES 6

typedef struct mem_chunk_header {
    size_t mch_sz; // requested allocation size
    LIST_ENTRY(mem_chunk_header) mch_mazfl_link; // ma_zone_t freelist link
} mem_chunk_header_t;

static_assert((1 << MA_SMALLEST_ZONE_ORDER) >= (sizeof(mem_chunk_header_t) * 2), "smallest zone size is < sizeof(mem_chunk_header_t) * 2");

typedef struct mem_chunk {
    mem_chunk_header_t mc_header;
    // uint8_t mc_data[];
} mem_chunk_t;

LIST_HEAD(mem_chunk_list, mem_chunk_header);
typedef struct mem_chunk_list mem_chunk_list_t;

typedef struct ma_zone_stats {
    size_t mzs_allocs;
    size_t mzs_frees;
} ma_zone_stats_t;

typedef struct ma_zone {
    size_t maz_mcsz; // mem chunk size not including mem chunk header
    lock_t maz_lock;
    size_t maz_npages;
    page_list_t maz_pagelist;
    ssize_t maz_nfree;
    mem_chunk_list_t maz_freelist;
    ma_zone_stats_t maz_stats;
} ma_zone_t;

size_t ma_zone_size(size_t znum);

void *ma_alloc(size_t size);
void ma_free(void *p);

void ma_teardown(void);

size_t ma_nalloced(void);

void ma_dump(bool verbose);
void ma_check(void);

#endif // _MEMORY_ALLOCATOR_H_
