#include <stdio.h>

#include "ods_types.h"
#include "block.h"
#include "on_disk_system.h"
#include "bcache.h"
#include "journal.h"

blk_phys_t *bl_phys(blk_t *b) {
    return b->bl_phys;
}

uint32_t bl_phys_type(blk_phys_t *bp) {
    return bp->bp_type;
}

void bl_phys_dump(blk_phys_t *bp) {
    printf("bp_type %s ", ods_type_to_string(bp->bp_type));
}

uint32_t bl_type(blk_t *b) {
    return bl_phys_type(bl_phys(b));
}

void bl_dump(blk_t *b) {
    printf("bl_blkno: %llu bl_refcount: %d%s bl_tid 0x%" PRIx64 " ", b->bl_blkno, b->bl_refcnt,
            (b->bl_flags & B_DIRTY) ? " B_DIRTY" : "", b->bl_tid);
    if (b->bl_bco_ops->bco_dump) {
        printf("bl_bco: ");
        b->bl_bco_ops->bco_dump(b->bl_bco);
    }
}
