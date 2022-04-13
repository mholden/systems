#ifndef _ON_DISK_SYSTEM_H_
#define _ON_DISK_SYSTEM_H_

#include "ods_types.h"
#include "on_disk_system_phys.h"

#define ODS_BLKSZ 1024 // fixed block size for now

struct on_disk_system {
    int ods_fd;
    bcache_t *ods_bc;
    jnl_t *ods_jnl;
    tx_mgr_t *ods_tm;
    uint64_t ods_blocks_start; // first address of ods blocks (first block past journal)
};

int ods_create(const char *path, uint64_t size);
int ods_startup(const char *path, ods_t **ods);
int ods_shutdown(ods_t *ods);

void ods_dump(ods_t *ods);

void ods_check(ods_t *ods); // runtime memory check
int ods_check_disk(const char *path); // offline disk check

const char *ods_type_to_string(uint32_t type);

void ods_bl_phys_dump(blk_phys_t *bp);

//
// note on lock ordering:
//  bc lock before tm lock
//  tm lock before jnl lock
//
// so:
//  1. bc lock
//  2. tm lock
//  3. jnl lock
//

#endif // _ON_DISK_SYSTEM_H_
