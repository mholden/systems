#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "on_disk_system.h"
#include "transaction.h"
#include "bcache.h"
#include "ods_block.h"
#include "journal.h"
#include "synch.h"

#define TEST_ODS_BACKING_FILE "/var/tmp/test_ods_file"
#define TEST_ODS_BLOCK_SIZE 1024
#define TEST_ODS_DEFAULT_FLSZ 64 * 1024 * ODS_BLKSZ
#define TEST_ODS_NF_BLOCKS (TEST_ODS_DEFAULT_FLSZ / TEST_ODS_BLOCK_SIZE)
#define TEST_ODS_NBGROUPS_BLOCKS (TEST_ODS_NF_BLOCKS - JNL_PHYS_DEFAULT_SIZE)
#define TEST_ODS_NBGROUPS (TEST_ODS_NBGROUPS_BLOCKS / ODS_BLOCKS_PER_GROUP)

// test journal wrap around
static void test_ods2(void) {
    ods_t *ods;
    tx_mgr_t *tm;
    bcache_t *bc;
    tid_t tid;
    uint64_t blkno, data;
    ods_blk_t *ob;
    ods_blk_phys_t *obp;
    
    printf("test_ods2\n");
    
    assert(ods_create(TEST_ODS_BACKING_FILE, TEST_ODS_DEFAULT_FLSZ) == 0);
    assert(ods_check_disk(TEST_ODS_BACKING_FILE) == 0);
    
    assert(ods_startup(TEST_ODS_BACKING_FILE, &ods) == 0);
    
    ods_check(ods);
    //ods_dump(ods);
    
    tm = ods->ods_tm;
    bc = ods->ods_bc;
    
    //
    // doing this 16 times will trigger journal wrap around
    //
    for (int j = 0; j < 16; j++) {
        assert(tx_start(tm, ODS_BLOCKS_PER_GROUP, &tid) == 0);
        
        ods_check(ods);
        //ods_dump(ods);
        
        // get and initialize all blocks in the block group
        blkno = ods->ods_blocks_start + (j * ODS_BLOCKS_PER_GROUP);
        data = (((uint64_t)rand() << 32) | (uint64_t)rand());
        for (int i = 0; i < ODS_BLOCKS_PER_GROUP; i++) {
            assert(bc_get(bc, blkno, &ob_bco_ops, (void **)&ob) == 0);
            obp = ob->ob_phys;
            obp->obp_blk.bp_type = ODS_PHYS_TYPE_BLOCK;
            obp->obp_data = data;
            bc_dirty(bc, ob_block(ob), tid);
            bc_release(bc, ob_block(ob));
            blkno++;
        }
        
        ods_check(ods);
        //ods_dump(ods);
        
        assert(tx_finish(tm, tid) == 0);
        
        ods_check(ods);
        //ods_dump(ods);
    }
    
    assert(ods_shutdown(ods) == 0);
    
    assert(ods_check_disk(TEST_ODS_BACKING_FILE) == 0);
}
 
//
// basic test: just write a transaction out
// and verify the disk is consistent
//
static void test_ods1(void) {
    ods_t *ods;
    tx_mgr_t *tm;
    bcache_t *bc;
    tid_t tid;
    uint64_t blkno, data;
    ods_blk_t *ob;
    ods_blk_phys_t *obp;
    
    printf("test_ods1\n");
    
    assert(ods_create(TEST_ODS_BACKING_FILE, TEST_ODS_DEFAULT_FLSZ) == 0);
    assert(ods_check_disk(TEST_ODS_BACKING_FILE) == 0);
    
    assert(ods_startup(TEST_ODS_BACKING_FILE, &ods) == 0);
    
    ods_check(ods);
    //ods_dump(ods);
    
    tm = ods->ods_tm;
    bc = ods->ods_bc;
    
    assert(tx_start(tm, ODS_BLOCKS_PER_GROUP, &tid) == 0);
    
    ods_check(ods);
    //ods_dump(ods);
    
    // get and initialize all blocks in the first block group
    blkno = ods->ods_blocks_start;
    data = (((uint64_t)rand() << 32) | (uint64_t)rand());
    for (int i = 0; i < ODS_BLOCKS_PER_GROUP; i++) {
        assert(bc_get(bc, blkno, &ob_bco_ops, (void **)&ob) == 0);
        obp = ob->ob_phys;
        obp->obp_blk.bp_type = ODS_PHYS_TYPE_BLOCK;
        obp->obp_data = data;
        bc_dirty(bc, ob_block(ob), tid);
        bc_release(bc, ob_block(ob));
        blkno++;
    }
    
    ods_check(ods);
    //ods_dump(ods);
    
    assert(tx_finish(tm, tid) == 0);
    
    ods_check(ods);
    //ods_dump(ods);
    
    assert(ods_shutdown(ods) == 0);
    
    assert(ods_check_disk(TEST_ODS_BACKING_FILE) == 0);
    
    // start back up
    assert(ods_startup(TEST_ODS_BACKING_FILE, &ods) == 0);
    ods_check(ods);
    
    bc = ods->ods_bc;
    
    // make sure things are as expected
    blkno = ods->ods_blocks_start;
    for (int i = 0; i < ODS_BLOCKS_PER_GROUP; i++) {
        assert(bc_get(bc, blkno, &ob_bco_ops, (void **)&ob) == 0);
        obp = ob->ob_phys;
        assert(obp->obp_blk.bp_type == ODS_PHYS_TYPE_BLOCK);
        if (i == 0)
            data = obp->obp_data;
        else
            assert(obp->obp_data == data);
        bc_release(bc, ob_block(ob));
        blkno++;
    }
    
    ods_check(ods);
    
    assert(ods_shutdown(ods) == 0);
    
    assert(ods_check_disk(TEST_ODS_BACKING_FILE) == 0);
}

typedef struct tor_thr_start_arg {
    thread_t *t;
    uint64_t *bgroups;
    ods_t *ods;
    int num_ops;
} tor_thr_start_arg_t;

static int tbr_thr_start(void *arg) {
    tor_thr_start_arg_t *targ = (tor_thr_start_arg_t *)arg;
    thread_t *t = targ->t;
    uint64_t *bgroups = targ->bgroups;
    ods_t *ods = targ->ods;
    int op, num_ops = targ->num_ops;
    uint64_t bgroup, blkno, data;
    ods_blk_t *ob0, *ob;
    ods_blk_phys_t *obp;
    tid_t tid;
    
    for (int i = 0; i < num_ops; i++) {
        op = rand() % 2;
        bgroup = rand() % TEST_ODS_NBGROUPS;
        blkno = ods->ods_blocks_start + (bgroup * ODS_BLOCKS_PER_GROUP);
        assert(bc_get(ods->ods_bc, blkno, &ob_bco_ops, (void **)&ob0) == 0);
        obp = ob_phys(ob0);
        if (op == 0) { // write
write:
            assert(ob_lock_exclusive(ob0) == 0);
            
            //printf("  thread %s writing to bgroup %llu (%" PRIu64 ") (iter %d)\n", t->t_name, bgroup, bgroups[bgroup], i);
            
            if (bgroups[bgroup])
                assert(obp->obp_data == bgroups[bgroup]);
            
            assert(tx_start(ods->ods_tm, ODS_BLOCKS_PER_GROUP, &tid) == 0);
            
            if (!bgroups[bgroup]) {
                obp->obp_blk.bp_type = ODS_PHYS_TYPE_BLOCK;
                obp->obp_data = 0;
            }
            obp->obp_data++;
            bc_dirty(ods->ods_bc, ob_block(ob0), tid);
            
            data = obp->obp_data;
            for (int j = 1; j < ODS_BLOCKS_PER_GROUP; j++) {
                assert(bc_get(ods->ods_bc, blkno + j, &ob_bco_ops, (void **)&ob) == 0);
                obp = ob_phys(ob);
                if (!bgroups[bgroup]) // bgroup is uninitialized
                    obp->obp_blk.bp_type = ODS_PHYS_TYPE_BLOCK;
                obp->obp_data = data;
                bc_dirty(ods->ods_bc, ob_block(ob), tid);
                bc_release(ods->ods_bc, ob_block(ob));
            }
            
            assert(tx_finish(ods->ods_tm, tid) == 0);
            
            bgroups[bgroup] = data;
        } else if (op == 1) { // read
            assert(ob_lock_shared(ob0) == 0);
            
            if (!bgroups[bgroup]) { // this bgroup hasn't been initialized yet
                assert(ob_unlock(ob0) == 0);
                goto write;
            }
            
            //printf("  thread %s reading bgroup %llu (%" PRIu64 ") (iter %d)\n", t->t_name, bgroup, bgroups[bgroup], i);
                
            assert(obp->obp_data == bgroups[bgroup]);
            
            for (int j = 1; j < ODS_BLOCKS_PER_GROUP; j++) {
                assert(bc_get(ods->ods_bc, blkno + j, &ob_bco_ops, (void **)&ob) == 0);
                obp = ob_phys(ob);
                assert(obp->obp_data == bgroups[bgroup]);
                bc_release(ods->ods_bc, ob_block(ob));
            }
        }
        assert(ob_unlock(ob0) == 0);
        bc_release(ods->ods_bc, ob_block(ob0));
        
        // flush every so often
        if ((bgroup % 128) == 0)
            assert(bc_flush(ods->ods_bc) == 0);
        
        //ods_check(ods);
#if 0
        if ((i % 256) == 0)
            printf("  thread %s through iter %d\n", t->t_name, i);
#endif
    }
    
    return 0;
}

static void test_ods_random(int num_ops) {
    ods_t *ods;
    char thr_name[5];;
    uint64_t *bgroups;
    thread_t *threads[8];
    tor_thr_start_arg_t targ[8];
    
    assert(bgroups = malloc(sizeof(uint64_t) * TEST_ODS_NBGROUPS));
    memset(bgroups, 0, sizeof(uint64_t) * TEST_ODS_NBGROUPS);
    
    printf("test_ods_random (num ops %d)\n", num_ops);
    
    assert(ods_create(TEST_ODS_BACKING_FILE, TEST_ODS_DEFAULT_FLSZ) == 0);
    assert(ods_check_disk(TEST_ODS_BACKING_FILE) == 0);
    
    assert(ods_startup(TEST_ODS_BACKING_FILE, &ods) == 0);
    
    ods_check(ods);
    //ods_dump(ods);
    
    // spawn 8 threads, have them each do num_ops / 8 random operations
    for (int i = 0; i < 8; i++) {
        sprintf(thr_name, "%s%d", "thr", i);
        assert(threads[i] = thread_create(thr_name));
        
        memset(&targ[i], 0, sizeof(tor_thr_start_arg_t));
        targ[i].t = threads[i];
        targ[i].bgroups = bgroups;
        targ[i].ods = ods;
        targ[i].num_ops = num_ops / 8;
        
        assert(thread_start(threads[i], tbr_thr_start, &targ[i]) == 0);
    }
    
    for (int i = 0; i < 8; i++)
        assert(thread_wait(threads[i], NULL) == 0);
    
    for (int i = 0; i < 8; i++)
        thread_destroy(threads[i]);
    
    ods_check(ods);
    
    assert(ods_shutdown(ods) == 0);
    
    assert(ods_check_disk(TEST_ODS_BACKING_FILE) == 0);
    
    free(bgroups);
}

#define DEFAULT_NUM_OPS 1024

int main(int argc, char **argv) {
    unsigned int seed = 0;
    int ch, num_ops = DEFAULT_NUM_OPS, fd;
    bool create = false;
    struct stat st;
    
    struct option longopts[] = {
        { "num",    required_argument,   NULL,   'n' },
        { "seed",   required_argument,   NULL,   's' },
        { "create", no_argument,         NULL,   'c' },
        { NULL,                0,        NULL,    0 }
    };

    while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (ch) {
            case 'n':
                num_ops = (int)strtol(optarg, NULL, 10);
                break;
            case 's':
                seed = (unsigned int)strtol(optarg, NULL, 10);
                break;
            case 'c':
                create = true;
                break;
            default:
                printf("usage: %s [--num <num-operations>] [--seed <seed>] [--create]\n", argv[0]);
                return -1;
        }
    }
    
    if (!seed) {
        seed = (unsigned int)time(NULL);
        srand(seed);
    } else
        srand(seed);
    
    printf("seed %u\n", seed);
    
    if (create) {
        // create the backing file if it's not already there
        fd = open(TEST_ODS_BACKING_FILE, O_CREAT | O_RDWR);
        assert(fd >= 0);
        close(fd);
    } else if (stat(TEST_ODS_BACKING_FILE, &st) == -1 && errno == ENOENT) {
        printf("%s doesn't exist: try running with --create\n", TEST_ODS_BACKING_FILE);
        return -1;
    }
    
    test_ods1();
    test_ods2();
    test_ods_random(num_ops);
    
    return 0;
}
