#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <sys/queue.h> // on macOS
#include <bsd/sys/queue.h> // on Linux

#include "ods_types.h"
#include "block_phys.h"
#include "synch.h"

#define B_DIRTY 0x0001 // block is dirty

struct block {
    rw_lock_t *bl_rwlock;
    uint64_t bl_blkno;
    int bl_refcnt;
    uint32_t bl_flags;
    void *bl_bco; // back pointer to 'block cache object'
    bco_ops_t *bl_bco_ops;
    tid_t bl_tid;
    LIST_ENTRY(block) bl_ht_link; // hash table link
    TAILQ_ENTRY(block) bl_fl_link; // free list link
    LIST_ENTRY(block) bl_dl_link; // dirty list link
    LIST_ENTRY(block) bl_tx_link; // transaction block list link
    blk_phys_t *bl_phys;
};

LIST_HEAD(block_list, block);
TAILQ_HEAD(block_tailq, block);

blk_phys_t *bl_phys(blk_t *b);
uint32_t bl_type(blk_t *b);
void bl_dump(blk_t *b);

#endif // _BLOCK_H_
