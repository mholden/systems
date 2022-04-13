#ifndef _ODS_BLOCK_H_
#define _ODS_BLOCK_H_

#include "ods_types.h"
#include "ods_block_phys.h"
#include "bcache.h"

struct ods_block {
    blk_t *ob_blk;
    ods_blk_phys_t *ob_phys;
};

blk_t *ob_block(ods_blk_t *ob);
ods_blk_phys_t *ob_phys(ods_blk_t *ob);

int ob_lock_exclusive(ods_blk_t *ob);
int ob_lock_shared(ods_blk_t *ob);
int ob_unlock(ods_blk_t *ob);

extern bco_ops_t ob_bco_ops;

#endif // _ODS_BLOCK_H_
