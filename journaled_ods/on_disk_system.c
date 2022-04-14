#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include "on_disk_system.h"
#include "journal.h"
#include "bcache.h"
#include "transaction.h"
#include "ods_block.h"

int ods_create(const char *path, uint64_t size) {
    struct stat st;
    int fd = -1, ret, err;
    
    ret = stat(path, &st);
    if (ret) {
        printf("ods_create: stat failed: %s\n", strerror(errno));
        err = errno;
        goto error_out;
    }
    
    if (!S_ISREG(st.st_mode) && !S_ISBLK(st.st_mode)) {
        printf("ods_create: invalid path (mode %d)\n", st.st_mode);
        err = EINVAL;
        goto error_out;
    }
    
    if (S_ISBLK(st.st_mode)) {
        printf("ods_create: block devices not yet supported\n");
        err = ENOTSUP;
        goto error_out;
    }
    
    if (size % ODS_BLKSZ) {
        printf("ods_create: invalid size (%" PRIu64 ")\n", size);
        err = EINVAL;
        goto error_out;
    }
    
    fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("ods_create: open failed: %s\n", strerror(errno));
        err = errno;
        goto error_out;
    }
    
    ret = ftruncate(fd, 0);
    if (ret) {
        printf("ods_create: ftruncate failed: %s\n", strerror(errno));
        err = errno;
        goto error_out;
    }
    
    err = posix_fallocate(fd, 0, size);
    if (err) {
        printf("ods_create: fallocate failed (%d size %" PRIu64 ")\n", err, size);
        goto error_out;
    }
    
    err = jnl_create(fd, ODS_JOURNAL_OFFSET, size);
    if (err)
        goto error_out;
    
    close(fd);
    
    return 0;
    
error_out:
    if (fd >= 0)
        close(fd);
        
    return err;
}

int ods_startup(const char *path, ods_t **ods) {
    struct stat st;
    ods_t *_ods = NULL;
    bool jnl_flushed = false;
    int ret, err;
    
    ret = stat(path, &st);
    if (ret) {
        printf("ods_startup: stat failed: %s\n", strerror(errno));
        err = errno;
        goto error_out;
    }
    
    if (!S_ISREG(st.st_mode) && !S_ISBLK(st.st_mode)) {
        printf("ods_startup: invalid path (mode %d)\n", st.st_mode);
        err = EINVAL;
        goto error_out;
    }
    
    _ods = malloc(sizeof(ods_t));
    if (!_ods) {
        err = ENOMEM;
        goto error_out;
    }
    
    memset(_ods, 0, sizeof(ods_t));
    
    _ods->ods_fd = open(path, O_RDWR);
    if (_ods->ods_fd < 0) {
        printf("ods_startup: couldn't open %s: %s\n", path, strerror(errno));
        goto error_out;
    }
    
    _ods->ods_bc = bc_create(_ods, ODS_BLKSZ, 256 * ODS_BLKSZ);
    if (!_ods->ods_bc) {
        printf("ods_startup: bc_create failed\n");
        goto error_out;
    }
    
    err = jnl_startup(_ods, &jnl_flushed);
    if (err)
        goto error_out;
    
    _ods->ods_blocks_start = ODS_JOURNAL_OFFSET + jnl_size(_ods->ods_jnl);
    
    err = tx_mgr_startup(_ods);
    if (err)
        goto error_out;
    
    //
    // if jnl_startup flushed the journal, let's do a
    // disk check here to make sure the disk looks good
    //
    if (jnl_flushed) {
        err = ods_check_disk(path);
        if (err)
            goto error_out;
    }
    
    *ods = _ods;
    
    return 0;
    
error_out:
    if (_ods) {
        if (_ods->ods_tm)
            tx_mgr_shutdown(_ods->ods_tm);
        if (_ods->ods_jnl)
            jnl_shutdown(_ods->ods_jnl);
        if (_ods->ods_bc)
            bc_destroy(_ods->ods_bc);
        if (_ods->ods_fd)
            close(_ods->ods_fd);
        free(_ods);
    }
    
    return err;
}

// TODO: synchronize this
int ods_shutdown(ods_t *ods) {
    bcache_t *bc;
    tx_mgr_t *tm;
    jnl_t *jnl;
    int err;
    
    bc = ods->ods_bc;
    tm = ods->ods_tm;
    jnl = ods->ods_jnl;
    
    err = bc_flush(bc);
    if (err)
        goto error_out;
    
    err = tx_mgr_shutdown(tm);
    if (err)
        goto error_out;
    
    err = jnl_shutdown(jnl);
    if (err)
        goto error_out;
    
    bc_destroy(bc);
    ods->ods_bc = NULL;
    
    return 0;
    
error_out:
    return err;
}

void ods_bl_phys_dump(blk_phys_t *bp) {
    switch (bl_phys_type(bp)) {
        case ODS_PHYS_TYPE_JOURNAL:
            jb_phys_dump((jnl_blk_phys_t *)bp);
            break;
        case ODS_PHYS_TYPE_BLOCK:
            ob_phys_dump((ods_blk_phys_t *)bp);
            break;
        default:
            printf("ods_bl_phys_dump: unknown block type %" PRIu32 "\n", bl_phys_type(bp));
            break;
    }
}

void ods_dump_locked(ods_t *ods) {
    printf("ods @ %p: ods_blocks_start %" PRIu64 "\n", ods, ods->ods_blocks_start);
    jnl_dump(ods->ods_jnl);
    tx_mgr_dump(ods->ods_tm);
    bc_dump(ods->ods_bc);
}

void ods_dump(ods_t *ods) {
    bcache_t *bc = ods->ods_bc;
    tx_mgr_t *tm = ods->ods_tm;
    tx_t *tx;
    jnl_t *jnl = ods->ods_jnl;
    
    // lock down the entire system
retry:
    lock_lock(bc->bc_lock);
    lock_lock(tm->tm_lock);
    tx = tm->tm_tx;
    if (tx->tx_state == TX_FLUSHING) {
        //
        // bc lock holder can't wait on tx
        // flush so synchronize it this way
        //
        lock_unlock(tm->tm_lock);
        lock_unlock(bc->bc_lock);
        goto retry;
    }
    lock_lock(jnl->j_lock);
    
    ods_dump_locked(ods);
    
    lock_unlock(jnl->j_lock);
    lock_unlock(tm->tm_lock);
    lock_unlock(bc->bc_lock);
}

void ods_check(ods_t *ods) {
    bcache_t *bc = ods->ods_bc;
    tx_mgr_t *tm = ods->ods_tm;
    tx_t *tx;
    jnl_t *jnl = ods->ods_jnl;
    
    // lock down the entire system
retry:
    lock_lock(bc->bc_lock);
    lock_lock(tm->tm_lock);
    tx = tm->tm_tx;
    if (tx->tx_state == TX_FLUSHING) {
        //
        // bc lock holder can't wait on tx
        // flush so synchronize it this way
        //
        lock_unlock(tm->tm_lock);
        lock_unlock(bc->bc_lock);
        goto retry;
    }
    lock_lock(jnl->j_lock);
    
    jnl_check(ods->ods_jnl);
    tx_mgr_check(ods->ods_tm);
    bc_check(ods->ods_bc);
    
    lock_unlock(jnl->j_lock);
    lock_unlock(tm->tm_lock);
    lock_unlock(bc->bc_lock);
}

int ods_check_disk(const char *path) {
    struct stat st;
    bool brand_new = false;
    uint64_t blocks_start, nblocks, nbgroups, blkno, obp_data;
    uint8_t *buf = NULL;
    ods_blk_phys_t *obp;
    int fd = -1, ret, err;
    ssize_t pret;
    
    ret = stat(path, &st);
    if (ret) {
        printf("ods_check_disk: stat failed: %s\n", strerror(errno));
        err = errno;
        goto error_out;
    }
    
    if (!S_ISREG(st.st_mode) && !S_ISBLK(st.st_mode)) {
        printf("ods_check_disk: invalid path (mode %d)\n", st.st_mode);
        err = EINVAL;
        goto error_out;
    }
    
    fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("ods_check_disk: open failed: %s\n", strerror(errno));
        err = errno;
        goto error_out;
    }
    
    buf = malloc(ODS_BLKSZ * ODS_BLOCKS_PER_GROUP);
    if (!buf) {
        err = ENOMEM;
        goto error_out;
    }
    
    err = jnl_check_disk(fd, ODS_JOURNAL_OFFSET, &brand_new);
    if (err)
        goto error_out;
    
    if (brand_new) // nothing more than journal has been written to disk yet, so don't check any other blocks
        goto out;

    // check the block groups
    blocks_start = ODS_JOURNAL_OFFSET + JNL_PHYS_DEFAULT_SIZE;
    nblocks = st.st_size / ODS_BLKSZ - blocks_start;
    nbgroups = nblocks / ODS_BLOCKS_PER_GROUP;
    
    blkno = blocks_start;
    for (uint64_t i = 0; i < nbgroups; i++) {
        pret = pread(fd, buf, ODS_BLKSZ * ODS_BLOCKS_PER_GROUP, blkno * ODS_BLKSZ);
        if (pret != (ODS_BLKSZ * ODS_BLOCKS_PER_GROUP)) {
            err = EIO;
            goto error_out;
        }
        obp = (ods_blk_phys_t *)buf;
        if (bl_phys_type((blk_phys_t *)obp) == ODS_PHYS_TYPE_BLOCK) { // don't check uninitialized block groups
            obp_data = obp->obp_data;
            for (int j = 1; j < ODS_BLOCKS_PER_GROUP; j++) {
                obp = (ods_blk_phys_t *)(buf + (j * ODS_BLKSZ));
                if (obp->obp_data != obp_data) {
                    printf("ods_check_disk: obp->obp_data != obp_data (block group %" PRIu64 ")", i);
                    err = EILSEQ;
                    // check all the block groups regardless of whether or not we find error
                }
            }
        }
        blkno += ODS_BLOCKS_PER_GROUP;
    }
    if (err)
        goto error_out;
    
out:
    close(fd);
    free(buf);

    return 0;
    
error_out:
    if (fd >= 0)
        close(fd);
    if (buf)
        free(buf);
    
    return err;
}

void ods_dump_disk(const char *path) {
    struct stat st;
    uint64_t blocks_start, nblocks, nbgroups, blkno, obp_data;
    uint8_t *buf = NULL;
    jnl_super_phys_t *jsp;
    ods_blk_phys_t *obp;
    int fd = -1, ret;
    ssize_t pret;
    
    ret = stat(path, &st);
    if (ret) {
        printf("ods_dump_disk: stat failed: %s\n", strerror(errno));
        goto error_out;
    }
    
    if (!S_ISREG(st.st_mode) && !S_ISBLK(st.st_mode)) {
        printf("ods_dump_disk: invalid path (mode %d)\n", st.st_mode);
        goto error_out;
    }
    
    fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("ods_dump_disk: open failed: %s\n", strerror(errno));
        goto error_out;
    }
    
    buf = malloc(ODS_BLKSZ * ODS_BLOCKS_PER_GROUP);
    if (!buf) {
        printf("ods_dump_disk: malloc failed on buf; bailing\n");
        goto error_out;
    }
    
    pret = pread(fd, buf, ODS_BLKSZ, ODS_JOURNAL_OFFSET * ODS_BLKSZ);
    if (pret != ODS_BLKSZ) {
        printf("ods_dump_disk: pread failed; bailing\n");
        goto error_out;
    }
    
    jsp = (jnl_super_phys_t *)buf;
    if (jsp->js_magic != JNL_PHYS_MAGIC) {
        printf("ods_dump_disk: jsp->js_magic != JNL_PHYS_MAGIC; bailing\n");
        goto error_out;
    }
    
    blocks_start = ODS_JOURNAL_OFFSET + JNL_PHYS_DEFAULT_SIZE;
    nblocks = st.st_size / ODS_BLKSZ - blocks_start;
    nbgroups = nblocks / ODS_BLOCKS_PER_GROUP;
    
    printf("ods @ %s: st_size %lld nblocks %" PRIu64 " nbgroups %" PRIu64 "\n", path, st.st_size, nblocks, nbgroups);
    
    jnl_dump_disk(fd);
    
    // dump any valid block groups
    printf("block groups:\n");
    blkno = blocks_start;
    for (uint64_t i = 0; i < nbgroups; i++) {
        pret = pread(fd, buf, ODS_BLKSZ * ODS_BLOCKS_PER_GROUP, blkno * ODS_BLKSZ);
        if (pret != (ODS_BLKSZ * ODS_BLOCKS_PER_GROUP)) {
            printf("ods_dump_disk: pread failed; bailing\n");
            goto error_out;
        }
        obp = (ods_blk_phys_t *)buf;
        if (bl_phys_type((blk_phys_t *)obp) == ODS_PHYS_TYPE_BLOCK) { // don't dump uninitialized block groups
            obp_data = obp->obp_data;
            printf(" bgroup[%" PRIu64 "]: obp_data %" PRIu64 " ", i, obp_data);
            for (int j = 1; j < ODS_BLOCKS_PER_GROUP; j++) {
                obp = (ods_blk_phys_t *)(buf + (j * ODS_BLKSZ));
                if (obp->obp_data != obp_data) {
                    printf(" ***** INCONSISTENT ***** ");
                    break;
                }
            }
            printf("\n");
        }
        blkno += ODS_BLOCKS_PER_GROUP;
    }
    
out:
    close(fd);
    free(buf);

    return;
    
error_out:
    if (fd >= 0)
        close(fd);
    if (buf)
        free(buf);
    
    return;
}

const char *ods_type_to_string(uint32_t type) {
    switch (type) {
        case ODS_PHYS_TYPE_JOURNAL:
            return "ODS_PHYS_TYPE_JOURNAL";
        case ODS_PHYS_TYPE_BLOCK:
            return "ODS_PHYS_TYPE_BLOCK";
        default:
            return "UNKNOWN";
    }
}
