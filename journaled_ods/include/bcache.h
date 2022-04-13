#ifndef _BLOCK_CACHE_H_
#define _BLOCK_CACHE_H_

#include <sys/queue.h> // on macOS
#include <bsd/sys/queue.h> // on Linux

#include "ods_types.h"
#include "synch.h"
#include "block.h"

// block cache (used to cache blocks either from a file or disk)

// 'block cache object' operations
struct bco_ops {
    int (*bco_init)(void **bco, blk_t *b);
    void (*bco_destroy)(void *bco);
    void (*bco_dump)(void *bco);
    void (*bco_check)(void *bco);
};

typedef struct bc_stats {
    uint64_t bcs_hits;
    uint64_t bcs_misses;
    uint64_t bcs_writes;
    uint64_t bcs_flushes;
} bc_stats_t;

struct bcache {
    ods_t *bc_ods;
    lock_t *bc_lock;
    int bc_fd;
    blk_list_t *bc_ht; // hash table
    blk_list_t bc_dl; // dirty list
    blk_tailq_t bc_fl; // free list
    uint32_t bc_blksz;
    uint32_t bc_currsz;
    uint32_t bc_maxsz;
    bc_stats_t bc_stats;
};

bcache_t *bc_create(ods_t *ods, uint32_t blksz, uint32_t maxsz);
void bc_destroy(bcache_t *bc);

int bc_get(bcache_t *bc, uint64_t blkno, bco_ops_t *bco_ops, void **bco);
void bc_dirty(bcache_t *bc, blk_t *b, tid_t tid);
void bc_release(bcache_t *bc, blk_t *b);

int bc_flush(bcache_t *bc);

void bc_dump(bcache_t *bc);
void bc_check(bcache_t *bc);

#endif /* _BLOCK_CACHE_H_ */
