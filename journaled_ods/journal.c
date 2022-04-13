#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "journal.h"
#include "on_disk_system.h"
#include "bcache.h"
#include "block.h"
#include "transaction.h"

int jnl_blk_init(void **bco, blk_t *b) {
    jnl_blk_t *jb;
    int err;
    
    jb = malloc(sizeof(jnl_blk_t));
    if (!jb) {
        err = ENOMEM;
        goto error_out;
    }
    memset(jb, 0, sizeof(jnl_blk_t));
    
    jb->jb_blk = b;
    jb->jb_phys = (jnl_blk_phys_t *)b->bl_phys;
    
    *bco = jb;
    
    return 0;
    
error_out:
    return err;
}

void jnl_blk_destroy(void *bco) {
    jnl_blk_t *jb = (jnl_blk_t *)bco;
    free(jb);
}

static void jnl_blk_dump_super(jnl_super_phys_t *jsp) {
    printf("js_blksz %" PRIu32 " js_size %" PRIu32 " js_flags 0x%" PRIx32 " js_next_tid %" PRIu64 " js_start_index %" PRIu32 " js_end_index %" PRIu32,
                jsp->js_blksz, jsp->js_size, jsp->js_flags, jsp->js_next_tid, jsp->js_start_index, jsp->js_end_index);
}

static void jnl_blk_dump_txd(jnl_txd_phys_t *jtd) {
    uint64_t *jtd_map = (uint64_t *)((uint8_t *)jtd + sizeof(jnl_txd_phys_t));
    printf("jtd_tid %" PRIu64 " jtd_size %" PRIu32 " jtd_map: ", jtd->jtd_tid, jtd->jtd_size);
    for (uint32_t i = 0; i < jtd->jtd_size; i++)
        printf("%" PRIu64 " ", jtd_map[i]);
}

static void jnl_blk_dump_txc(jnl_txc_phys_t *jtc) {
    printf("jtc_tid: %" PRIu64 " ", jtc->jtc_tid);
}

void jb_phys_dump(jnl_blk_phys_t *jbp) {
    blk_phys_t *bp = (blk_phys_t *)jbp;
    bl_phys_dump(bp);
    printf("jhp_type: %s ", jnl_type_to_string(jbp->jhp_type));
    if (jbp->jhp_type == JNL_PHYS_TYPE_SUPER)
        jnl_blk_dump_super((jnl_super_phys_t *)jbp);
    else if (jbp->jhp_type == JNL_PHYS_TYPE_TX_DESC)
        jnl_blk_dump_txd((jnl_txd_phys_t *)jbp);
    else // JNL_PHYS_TYPE_TX_COMMIT
        jnl_blk_dump_txc((jnl_txc_phys_t *)jbp);
}

void jnl_blk_dump(void *bco) {
    jnl_blk_t *jb = (jnl_blk_t *)bco;
    jb_phys_dump(jb->jb_phys);
}

static void jnl_blk_check_super(jnl_super_phys_t *jsp) {
    assert(jsp->js_magic == JNL_PHYS_MAGIC);
    assert((jsp->js_flags & ~(JNLP_JSF_VALID_FLAGS)) == 0);
    assert(jsp->js_size == JNL_PHYS_DEFAULT_SIZE);
    assert(jsp->js_blksz == ODS_BLKSZ);
    assert((jsp->js_start_index >= 1) && (jsp->js_start_index < jsp->js_size));
    assert((jsp->js_end_index >= 1) && (jsp->js_end_index < jsp->js_size));
}

static void jnl_blk_check_txd(jnl_txd_phys_t *jtd) {
    assert(0);
}

static void jnl_blk_check_txc(jnl_txc_phys_t *jtc) {
    assert(0);
}

void jnl_blk_check(void *bco) {
    jnl_blk_t *jb = (jnl_blk_t *)bco;
    jnl_blk_phys_t *jbp = (jnl_blk_phys_t *)jb->jb_phys;
    
    assert(bl_type(jb_block(jb)) == ODS_PHYS_TYPE_JOURNAL);
    assert((jbp->jhp_type == JNL_PHYS_TYPE_SUPER) || (jbp->jhp_type == JNL_PHYS_TYPE_TX_DESC) ||
            (jbp->jhp_type == JNL_PHYS_TYPE_TX_COMMIT));
    
    if (jbp->jhp_type == JNL_PHYS_TYPE_SUPER)
        jnl_blk_check_super((jnl_super_phys_t *)jbp);
    else if (jbp->jhp_type == JNL_PHYS_TYPE_TX_DESC)
        jnl_blk_check_txd((jnl_txd_phys_t *)jbp);
    else // JNL_PHYS_TYPE_TX_COMMIT
        jnl_blk_check_txc((jnl_txc_phys_t *)jbp);
    
    return;
}

bco_ops_t jb_bco_ops = {
    .bco_init = jnl_blk_init,
    .bco_destroy = jnl_blk_destroy,
    .bco_dump = jnl_blk_dump,
    .bco_check = jnl_blk_check
};

int jnl_create(int fd, uint64_t offset, size_t dsksz) {
    blk_phys_t *blkp = NULL;
    jnl_super_phys_t *jnlsp;
    int err;
    ssize_t ret;
    
    if (dsksz <= JNL_PHYS_DEFAULT_SIZE) {
        err = EINVAL;
        goto error_out;
    }
    
    blkp = malloc(ODS_BLKSZ);
    if (!blkp) {
        err = ENOMEM;
        goto error_out;
    }
    
    memset(blkp, 0, ODS_BLKSZ);
    blkp->bp_type = ODS_PHYS_TYPE_JOURNAL;
    
    jnlsp = (jnl_super_phys_t *)blkp;
    jnlsp->js_header.jhp_type = JNL_PHYS_TYPE_SUPER;
    jnlsp->js_flags = JNLP_JSF_BRAND_NEW;
    jnlsp->js_magic = JNL_PHYS_MAGIC;
    jnlsp->js_size = JNL_PHYS_DEFAULT_SIZE;
    jnlsp->js_start_index = 1;
    jnlsp->js_end_index = 1;
    jnlsp->js_blksz = ODS_BLKSZ;
    
    ret = pwrite(fd, blkp, ODS_BLKSZ, offset * ODS_BLKSZ);
    if (ret != ODS_BLKSZ) {
        err = EIO;
        goto error_out;
    }
    
    free(blkp);
    
    return 0;
    
error_out:
    if (blkp)
        free(blkp);
    
    return err;
}

int jnl_startup(ods_t *ods) {
    jnl_t *jnl;
    bcache_t *bc;
    int err;
    
    jnl = malloc(sizeof(jnl_t));
    if (!jnl) {
        err = ENOMEM;
        goto error_out;
    }
    
    memset(jnl, 0, sizeof(jnl_t));
    
    jnl->j_ods = ods;
    bc = ods->ods_bc;
    
    // read in the journal super block
    err = bc_get(bc, ODS_JOURNAL_OFFSET, &jb_bco_ops, (void **)&jnl->j_super);
    if (err)
        goto error_out;
    
    jnl->j_lock = lock_create();
    if (!jnl->j_lock) {
        err = ENOMEM;
        goto error_out;
    }
    
    ods->ods_jnl = jnl;
    
    return 0;
    
error_out:
    if (jnl) {
        if (jnl->j_super)
            bc_release(bc, jb_block(jnl->j_super));
        free(jnl);
    }
    
    return err;
}

int jnl_shutdown(jnl_t *jnl) {
    ods_t *ods;
    int err;
    
    ods = jnl->j_ods;
    
    err = jnl_flush(jnl);
    if (err)
        goto error_out;
    
    //jnl_dump(jnl);
    
    err = lock_destroy(jnl->j_lock);
    if (err)
        goto error_out;
    
    bc_release(ods->ods_bc, jb_block(jnl->j_super));
    
    free(jnl);
    ods->ods_jnl = NULL;
    
    return 0;
    
error_out:
    return err;
}

int jnl_check_disk(int fd, uint64_t offset, bool *brand_new) {
    blk_phys_t *blkp;
    jnl_super_phys_t *jnlsp;
    jnl_txd_phys_t *jtdp;
    jnl_txc_phys_t *jtcp;
    ssize_t ret;
    int err;
    
    blkp = malloc(ODS_BLKSZ);
    if (!blkp) {
        err = ENOMEM;
        goto error_out;
    }
    
    ret = pread(fd, blkp, ODS_BLKSZ, ODS_JOURNAL_OFFSET);
    if (ret != ODS_BLKSZ) {
        printf("jnl_check_disk: pread failed\n");
        err = EIO;
        goto error_out;
    }
    
    // check the block header
    if (blkp->bp_type != ODS_PHYS_TYPE_JOURNAL) {
        printf("jnl_check_disk: blkp->bp_header.bhp_type != ODS_PHYS_TYPE_JOURNAL\n");
        err = EILSEQ;
        goto error_out;
    }
    
    // check the journal super block
    jnlsp = (jnl_super_phys_t *)blkp;
    
    if (jnlsp->js_header.jhp_type != JNL_PHYS_TYPE_SUPER) {
        printf("jnl_check_disk: jnlsp->js_header.jhp_type != JNL_PHYS_TYPE_SUPER\n");
        err = EILSEQ;
        goto error_out;
    }
    
    if (jnlsp->js_magic != JNL_PHYS_MAGIC) {
        printf("jnl_check_disk: jnlsp->js_magic != JNL_PHYS_MAGIC\n");
        err = EILSEQ;
        goto error_out;
    }
    
    if ((jnlsp->js_flags & ~(JNLP_JSF_VALID_FLAGS)) != 0) {
        printf("jnl_check_disk: (jnlsp->js_flags & ~(JNLP_JSF_VALID_FLAGS)) != 0\n");
        err = EILSEQ;
        goto error_out;
    }
    
    if (jnlsp->js_blksz != ODS_BLKSZ) {
        printf("jnl_check_disk: jnlsp->js_blksz != ODS_BLKSZ\n");
        err = EILSEQ;
        goto error_out;
    }
    
    if (jnlsp->js_size != JNL_PHYS_DEFAULT_SIZE) {
        printf("jnl_check_disk: jnlsp->js_size != JNL_PHYS_DEFAULT_SIZE\n");
        err = EILSEQ;
        goto error_out;
    }
    
    if ((jnlsp->js_start_index < 1) || (jnlsp->js_start_index > (jnlsp->js_size - 1))) {
        printf("jnl_check_disk: (jnlsp->js_start_index < 1) || (jnlsp->js_start_index > (jnlsp->js_size - 1)\n");
        err = EILSEQ;
        goto error_out;
    }
    
    if ((jnlsp->js_end_index < 1) || (jnlsp->js_end_index > (jnlsp->js_size - 1))) {
        printf("jnl_check_disk: (jnlsp->js_end_index < 1) || (jnlsp->js_end_index > (jnlsp->js_size - 1)\n");
        err = EILSEQ;
        goto error_out;
    }
    
    if (jnlsp->js_flags & JNLP_JSF_BRAND_NEW) {
        if ((jnlsp->js_start_index != 1) || (jnlsp->js_end_index != 1)) {
            err = EILSEQ;
            goto error_out;
        }
        *brand_new = true;
    }
    
    free(blkp);
    
    return 0;
    
error_out:
    if (blkp)
        free(blkp);
        
    return err;
}

const char *jnl_type_to_string(uint32_t type) {
    switch (type) {
        case JNL_PHYS_TYPE_SUPER:
            return "JNL_PHYS_TYPE_SUPER";
        case JNL_PHYS_TYPE_TX_DESC:
            return "JNL_PHYS_TYPE_TX_DESC";
        case JNL_PHYS_TYPE_TX_COMMIT:
            return "JNL_PHYS_TYPE_TX_COMMIT";
        default:
            return "UNKNOWN";
    }
}

jnl_blk_phys_t *jb_phys(jnl_blk_t *jb) {
    return jb->jb_phys;
}

uint32_t jb_phys_type(jnl_blk_phys_t *jbp) {
    return jbp->jhp_type;
}

tid_t jnl_get_next_tid(jnl_t *jnl) {
    jnl_super_phys_t *jsp;
    tid_t next_tid;
    
    lock_lock(jnl->j_lock);
    
    jsp = (jnl_super_phys_t *)jb_phys(jnl->j_super);
    next_tid = jsp->js_next_tid++;
    
    lock_unlock(jnl->j_lock);
    
    return next_tid;
}

// called with jnl lock held
static uint32_t jnl_free_space(jnl_t *jnl) {
    jnl_super_phys_t *jsp = (jnl_super_phys_t *)bl_phys(jb_block(jnl->j_super));
    uint32_t jnl_blks_used, jnl_blks_unused;
    
    if (jsp->js_end_index >= jsp->js_start_index) {
        jnl_blks_used = (jsp->js_end_index - jsp->js_start_index + 1);
        jnl_blks_unused = jsp->js_size - jnl_blks_used;
    } else { // wrap around case
        jnl_blks_unused = (jsp->js_start_index - jsp->js_end_index - 1);
    }
    
    return jnl_blks_unused;
}

static int jnl_flush_locked(jnl_t *jnl) {
    ods_t *ods;
    jnl_super_phys_t *jsp;
    jnl_txd_phys_t *td;
    jnl_txc_phys_t *tc;
    uint8_t *mbuf = NULL, *dbuf = NULL;
    uint32_t tx_start, curr_ind, ntxblocks;
    int32_t nblocks;
    uint64_t *td_map;
    ssize_t ret;
    int err;
    
    ods = jnl->j_ods;
    jsp = (jnl_super_phys_t *)bl_phys(jb_block(jnl->j_super));
    if (jsp->js_start_index == jsp->js_end_index) // nothing to do
        goto out;
    
    // meta data buffer
    mbuf = malloc(jsp->js_blksz * 2);
    if (!mbuf)
        goto error_out;
    
    // data buffer
    dbuf = malloc(jsp->js_blksz);
    if (!mbuf)
        goto error_out;

    
    if (jsp->js_end_index > jsp->js_start_index)
        nblocks = jsp->js_end_index - jsp->js_start_index + 1;
    else
        nblocks = (jsp->js_size - jsp->js_start_index) + jsp->js_end_index;
    
    assert((nblocks > 2) && (nblocks < jsp->js_size));
    
    // read through the journal and write out each transaction
    
    tx_start = jsp->js_start_index;
    while (nblocks) {
        // read in the transaction descriptor
        ret = pread(ods->ods_fd, mbuf, jsp->js_blksz, (ODS_JOURNAL_OFFSET + tx_start) * jsp->js_blksz);
        if (ret != jsp->js_blksz) {
            err = EIO;
            goto error_out;
        }
        
        td = (jnl_txd_phys_t *)mbuf;
        if ((bl_phys_type((blk_phys_t *)td) != ODS_PHYS_TYPE_JOURNAL) ||
                (jb_phys_type((jnl_blk_phys_t *)td) != JNL_PHYS_TYPE_TX_DESC)) {
            printf("jnl_flush_locked: tx descriptor block is corrupt; bailing\n");
            err = EILSEQ;
            goto write_out_super;
        }
        
        if (td->jtd_size > (nblocks - 1)) {
            printf("jnl_flush_locked: tx descriptor size is invalid (%" PRIu32 "); bailing\n", td->jtd_size);
            err = EILSEQ;
            goto write_out_super;
        }
        
        // now read in the transaction commit block and make sure its tid matches the descriptor
        ret = pread(ods->ods_fd, mbuf + jsp->js_blksz, jsp->js_blksz, (ODS_JOURNAL_OFFSET + tx_start + td->jtd_size + 1) * jsp->js_blksz);
        if (ret != jsp->js_blksz) {
            err = EIO;
            goto error_out;
        }
        
        tc = (jnl_txc_phys_t *)(mbuf + jsp->js_blksz);
        if ((bl_phys_type((blk_phys_t *)tc) != ODS_PHYS_TYPE_JOURNAL) ||
                (jb_phys_type((jnl_blk_phys_t *)tc) != JNL_PHYS_TYPE_TX_COMMIT)) {
            printf("jnl_flush_locked: tx commit block is corrupt; bailing\n");
            err = EILSEQ;
            goto write_out_super;
        }
        
        if (tc->jtc_tid != td->jtd_tid) {
            printf("jnl_flush_locked: tx commit tid (%" PRIu64 ") / descriptor tid (%" PRIu64 ") mismatch; bailing\n", tc->jtc_tid, td->jtd_tid);
            err = EILSEQ;
            goto write_out_super;
        }
        
        // ok, looks like this is a valid transaction. write out all the data blocks
        td_map = (uint64_t *)((uint8_t *)td + sizeof(jnl_txd_phys_t));
        curr_ind = tx_start + 1;
        for (uint32_t i = 0; i < td->jtd_size; i++) {
            // read from the journal
            ret = pread(ods->ods_fd, dbuf, jsp->js_blksz, (ODS_JOURNAL_OFFSET + curr_ind) * jsp->js_blksz);
            if (ret != jsp->js_blksz) {
                err = EIO;
                goto error_out;
            }
            // write to it's actual location
            ret = pwrite(ods->ods_fd, dbuf, jsp->js_blksz, td_map[i] * jsp->js_blksz);
            if (ret != jsp->js_blksz) {
                err = EIO;
                goto error_out;
            }
            curr_ind++;
            if (curr_ind >= jsp->js_size) // wrap around
                curr_ind = (curr_ind % jsp->js_size) + 1;
        }
        
        ntxblocks = td->jtd_size + 2;
        tx_start += ntxblocks;
        if (tx_start >= jsp->js_size) // wrap around
            tx_start = (tx_start % jsp->js_size) + 1;
        nblocks -= ntxblocks;
    }
    assert(nblocks == 0);
    
write_out_super:
    jsp->js_start_index = jsp->js_end_index;
    ret = pwrite(ods->ods_fd, jsp, jsp->js_blksz, ODS_JOURNAL_OFFSET);
    if (ret != jsp->js_blksz) {
        err = EIO;
        goto error_out;
    }
    if (err) // might have been set above on corruption
        goto error_out;
    
out:
    free(mbuf);
    free(dbuf);
    
    return 0;
    
error_out:
    if (mbuf)
        free(mbuf);
    if (dbuf)
        free(dbuf);
    
    return err;
}

//
// flushes all transactions in the journal from
// the journal to their actual locations on disk
//
int jnl_flush(jnl_t *jnl) {
    int err;
    
    lock_lock(jnl->j_lock);
    err = jnl_flush_locked(jnl);
    lock_unlock(jnl->j_lock);
    
    return err;
}

int jnl_write_transaction(jnl_t *jnl, blk_list_t *bl, uint32_t ntxblocks, tid_t tid) {
    ods_t *ods;
    jnl_super_phys_t *jsp;
    jnl_txd_phys_t *td;
    jnl_txc_phys_t *tc;
    uint8_t *buf, *p;
    uint32_t nblocks, free_space, tail_space, head_space, tail_start, head_start;
    uint64_t *tdmap;
    blk_t *b;
    ssize_t ret;
    int err, i;
    
    lock_lock(jnl->j_lock);
    
    ods = jnl->j_ods;
    jsp = (jnl_super_phys_t *)bl_phys(jb_block(jnl->j_super));
    nblocks = ntxblocks + 2; // ntxblock + tx descriptor block + tx commit block
    
    if (ntxblocks < 1) {
        err = EINVAL;
        goto error_out;
    }
    
    if (nblocks > (jsp->js_size - 1)) {
        err = E2BIG;
        goto error_out;
    }
    
    //
    // make sure we have enough free space in the journal.
    // if we don't, then flush the journal to free up space
    //
    free_space = jnl_free_space(jnl);
    if (nblocks > free_space) {
        err = jnl_flush_locked(jnl);
        if (err)
            goto error_out;
        free_space = jnl_free_space(jnl);
        assert(free_space == (jsp->js_size - 1));
    }
    assert(free_space >= nblocks);
    
    buf = malloc(jsp->js_blksz * nblocks);
    if (!buf) {
        err = ENOMEM;
        goto error_out;
    }
    p = buf;
    
    // set up transaction descriptor block
    td = (jnl_txd_phys_t *)p;
    memset(td, 0, jsp->js_blksz);
    td->jtd_header.jhp_blk.bp_type = ODS_PHYS_TYPE_JOURNAL;
    td->jtd_header.jhp_type = JNL_PHYS_TYPE_TX_DESC;
    td->jtd_size = ntxblocks;
    td->jtd_tid = tid;
    tdmap = (uint64_t *)(p + sizeof(jnl_txd_phys_t));
    
    // write the blocks in the transaction into our buffer
    p += jsp->js_blksz;
    i = 0;
    LIST_FOREACH(b, bl, bl_tx_link) {
        memcpy(p, bl_phys(b), jsp->js_blksz);
        p += jsp->js_blksz;
        tdmap[i++] = b->bl_blkno;
    }
    
    // set up transaction commit block
    tc = (jnl_txc_phys_t *)p;
    memset(tc, 0, jsp->js_blksz);
    tc->jtc_header.jhp_blk.bp_type = ODS_PHYS_TYPE_JOURNAL;
    tc->jtc_header.jhp_type = JNL_PHYS_TYPE_TX_COMMIT;
    tc->jtc_tid = tid;
    
    // figure out where the free space lies in the journal
    if (jsp->js_end_index >= jsp->js_start_index) {
        tail_space = (jsp->js_size - 1) - jsp->js_end_index;
        tail_start = ODS_JOURNAL_OFFSET + jsp->js_end_index + 1;
        if (jsp->js_end_index == jsp->js_start_index) {
            tail_space++;
            tail_start--;
        }
        head_space = jsp->js_start_index - 1;
        head_start = ODS_JOURNAL_OFFSET + 1;
    } else {
        tail_space = (jsp->js_start_index - jsp->js_end_index) - 1;
        head_space = 0; // no head space in this case
    }
    assert((tail_space + head_space) == free_space);

    // now write the transaction out
    p = buf;
    if (tail_space >= nblocks) {
        ret = pwrite(ods->ods_fd, p, nblocks * jsp->js_blksz, tail_start * jsp->js_blksz);
        if (ret != (nblocks * jsp->js_blksz)) {
            err = EIO;
            goto error_out;
        }
    } else { // we need the head space, too
        assert(head_space);
        if (tail_space) {
            ret = pwrite(ods->ods_fd, p, tail_space * jsp->js_blksz, tail_start * jsp->js_blksz);
            if (ret != (tail_space * jsp->js_blksz)) {
                err = EIO;
                goto error_out;
            }
        }
        p += tail_space * jsp->js_blksz;
        ret = pwrite(ods->ods_fd, p, (nblocks - tail_space) * jsp->js_blksz, head_start * jsp->js_blksz);
        if (ret != ((nblocks - tail_space) * jsp->js_blksz)) {
            err = EIO;
            goto error_out;
        }
    }
    
    if (jsp->js_end_index == jsp->js_start_index)
        jsp->js_end_index = jsp->js_end_index + nblocks - 1;
    else
        jsp->js_end_index = jsp->js_end_index + nblocks;
    
    if (jsp->js_end_index >= jsp->js_size) // wrap around
        jsp->js_end_index = (jsp->js_end_index % jsp->js_size) + 1;
    
    if (jsp->js_flags & JNLP_JSF_BRAND_NEW)
        jsp->js_flags &= ~(JNLP_JSF_BRAND_NEW);
    
    // and finally, write out the super block
    ret = pwrite(ods->ods_fd, jsp, jsp->js_blksz, ODS_JOURNAL_OFFSET * jsp->js_blksz);
    if (ret != jsp->js_blksz) {
        err = EIO;
        goto error_out;
    }
    
    // sync it all out
    ret = fsync(ods->ods_fd);
    if (ret) {
        err = errno;
        goto error_out;
    }
    
    free(buf);
    lock_unlock(jnl->j_lock);
    
    return 0;
    
error_out:
    if (buf)
        free(buf);
    
    lock_unlock(jnl->j_lock);
    
    return err;
}

// appropriate locks must be held
void jnl_dump(jnl_t *jnl) {
    ods_t *ods = jnl->j_ods;
    jnl_super_phys_t *jsp = (jnl_super_phys_t *)bl_phys(jb_block(jnl->j_super));
    printf("jnl @ %p: js_blksz %" PRIu32 " js_size %" PRIu32 " js_flags 0x%" PRIx32 " js_next_tid %" PRIu64
           " js_start_index %" PRIu32 " js_end_index %" PRIu32 " (free space %" PRIu32 ")\n",
            jnl, jsp->js_blksz, jsp->js_size, jsp->js_flags, jsp->js_next_tid, jsp->js_start_index, jsp->js_end_index, jnl_free_space(jnl));
    jnl_dump_disk(ods->ods_fd);
}

void jnl_dump_disk(int fd) {
    uint8_t *buf = NULL;
    jnl_super_phys_t *jsp;
    uint32_t jsize, curr_ind, nblocks;
    ssize_t ret;
    
    buf = malloc(ODS_BLKSZ);
    if (!buf) {
        printf("jnl_dump_disk: couldn't allocate memory for buf\n");
        goto out;
    }
    
    ret = pread(fd, buf, ODS_BLKSZ, ODS_JOURNAL_OFFSET * ODS_BLKSZ);
    if (ret != ODS_BLKSZ) {
        printf("jnl_dump_disk: couldn't read in journal super\n");
        goto out;
    }
    
    printf("jnl on disk @ block %d: \n", ODS_JOURNAL_OFFSET);
    
    printf(" super: ");
    jb_phys_dump((jnl_blk_phys_t *)buf);
    printf("\n");
    
    jsp = (jnl_super_phys_t *)buf;
    jsize = jsp->js_size;
    
    if (jsp->js_end_index == jsp->js_start_index) // journal is empty
        goto out;
    
    if (jsp->js_end_index > jsp->js_start_index)
        nblocks = jsp->js_end_index - jsp->js_start_index + 1;
    else
        nblocks = (jsize - 1) - (jsp->js_start_index - jsp->js_end_index - 1);
    
    curr_ind = jsp->js_start_index;
    while (nblocks) {
        ret = pread(fd, buf, ODS_BLKSZ, (ODS_JOURNAL_OFFSET + curr_ind) * ODS_BLKSZ);
        if (ret != ODS_BLKSZ) {
            printf("jnl_dump_disk: couldn't read in journal block %" PRIu32 "\n", curr_ind);
            goto out;
        }
        printf(" jnl[%" PRIu32 "]: ", curr_ind);
        ods_bl_phys_dump((blk_phys_t *)buf);
        printf("\n");
        curr_ind++;
        nblocks--;
    }
    
out:
    if (buf)
        free(buf);
    
    return;
}

// appropriate locks must be held
void jnl_check(jnl_t *jnl) {
    jnl_super_phys_t *jsp = (jnl_super_phys_t *)bl_phys(jb_block(jnl->j_super));
    assert(jsp->js_magic == JNL_PHYS_MAGIC);
    assert(!(jsp->js_flags & ~JNLP_JSF_VALID_FLAGS));
    if (jsp->js_flags & JNLP_JSF_BRAND_NEW)
        assert((jsp->js_start_index == 1) && (jsp->js_end_index == 1));
    assert(jsp->js_size == JNL_PHYS_DEFAULT_SIZE);
    assert(jsp->js_blksz == ODS_BLKSZ);
    assert(jsp->js_next_tid != TX_ID_NONE);
    assert((jsp->js_start_index >= 1) && (jsp->js_start_index < jsp->js_size));
    assert((jsp->js_end_index >= 1) && (jsp->js_end_index < jsp->js_size));
    // we never mark jsp as dirty and we don't modify it through the normal transaction-path
    assert(!(jb_block(jnl->j_super)->bl_flags & B_DIRTY));
    assert(jb_block(jnl->j_super)->bl_tid == TX_ID_NONE);
}

uint32_t jnl_size(jnl_t *jnl) {
    jnl_super_phys_t *jsp = (jnl_super_phys_t *)jnl->j_super->jb_phys;
    return jsp->js_size;
}

blk_t *jb_block(jnl_blk_t *jb) {
    return jb->jb_blk;
}




