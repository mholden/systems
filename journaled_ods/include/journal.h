#ifndef _JOURNAL_H_
#define _JOURNAL_H_

#include "ods_types.h"
#include "journal_phys.h"
#include "bcache.h"
#include "synch.h"

struct jnl_blk {
    blk_t *jb_blk;
    jnl_blk_phys_t *jb_phys;
};

blk_t *jb_block(jnl_blk_t *jb);
jnl_blk_phys_t *jb_phys(jnl_blk_t *jb);

extern bco_ops_t jb_bco_ops;

struct journal {
    ods_t *j_ods;
    lock_t *j_lock;
    jnl_blk_t *j_super;
};

int jnl_create(int fd, uint64_t offset, size_t dsksz);
int jnl_startup(ods_t *ods);
int jnl_shutdown(jnl_t *jnl);

uint32_t jnl_size(jnl_t *jnl);
tid_t jnl_get_next_tid(jnl_t *jnl);

int jnl_write_transaction(jnl_t *jnl, blk_list_t *bl, uint32_t nblocks, tid_t tid);
int jnl_flush(jnl_t *jnl);

void jnl_check(jnl_t *jnl);
void jnl_dump(jnl_t *jnl);
void jnl_dump_disk(int fd);

void jnl_check(jnl_t *jnl);
int jnl_check_disk(int fd, uint64_t offset, bool *brand_new);

const char *jnl_type_to_string(uint32_t type);

#endif // _JOURNAL_H_
