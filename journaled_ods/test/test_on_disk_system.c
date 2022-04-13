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

#include "on_disk_system.h"
#include "transaction.h"
#include "bcache.h"
#include "ods_block.h"

#define TEST_ODS_BACKING_FILE "/var/tmp/test_ods_file"
#define TEST_ODS_BLOCK_SIZE 1024
#define TEST_ODS_DEFAULT_FLSZ 64 * 1024 * ODS_BLKSZ

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

#define DEFAULT_NUM_OPS 1024

int main(int argc, char **argv) {
    unsigned int seed = 0;
    int ch, num_ops = DEFAULT_NUM_OPS, fd;
    bool create = false;
    
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
                printf("usage: %s [--num <num-elements>] [--seed <seed>] [--create]\n", argv[0]);
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
        
    }
    
    test_ods1();
    
    return 0;
}
