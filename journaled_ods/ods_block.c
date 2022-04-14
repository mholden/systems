#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ods_block.h"
#include "block.h"
#include "bcache.h"
#include "on_disk_system.h"

blk_t *ob_block(ods_blk_t *ob) {
    return ob->ob_blk;
}

ods_blk_phys_t *ob_phys(ods_blk_t *ob) {
    return ob->ob_phys;
}

int ob_lock_exclusive(ods_blk_t *ob) {
    return rwl_lock_exclusive(ob_block(ob)->bl_rwlock);
}

int ob_lock_shared(ods_blk_t *ob) {
    return rwl_lock_shared(ob_block(ob)->bl_rwlock);
}

int ob_unlock(ods_blk_t *ob) {
    return rwl_unlock(ob_block(ob)->bl_rwlock);
}

int ods_blk_init(void **bco, blk_t *b) {
    ods_blk_t *ob;
    int err;
    
    ob = malloc(sizeof(ods_blk_t));
    if (!ob) {
        err = ENOMEM;
        goto error_out;
    }
    memset(ob, 0, sizeof(ods_blk_t));
    
    ob->ob_blk = b;
    ob->ob_phys = (ods_blk_phys_t *)b->bl_phys;
    
    *bco = ob;
    
    return 0;
    
error_out:
    return err;
}

void ods_blk_destroy(void *bco) {
    ods_blk_t *ob = (ods_blk_t *)bco;
    free(ob);
}

void ob_phys_dump(ods_blk_phys_t *obp) {
    bl_phys_dump((blk_phys_t *)obp);
    printf("obp_data: %" PRIu64 " ", obp->obp_data);
}

void ods_blk_dump(void *bco) {
    ods_blk_t *ob = (ods_blk_t *)bco;
    ob_phys_dump(ob->ob_phys);
}

bco_ops_t ob_bco_ops = {
    .bco_init = ods_blk_init,
    .bco_destroy = ods_blk_destroy,
    .bco_dump = ods_blk_dump,
    .bco_check = NULL
};
