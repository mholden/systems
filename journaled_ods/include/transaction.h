#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include <stdint.h>

#include "ods_types.h"
#include "synch.h"
#include "block.h"

#define TX_ID_NONE (tid_t)(-1LL)

// tx_state states
#define TX_IDLE     0
#define TX_OPEN     1
#define TX_FLUSHING 2

struct tx {
    tid_t tx_id;
    uint32_t tx_state;
    uint32_t tx_refs;
    blk_list_t tx_blks;
    uint32_t tx_nblks; // number of actual blocks in the transaction
    //
    // 'predicted' maximum number of blocks that will
    // be in the transaction based on nblocks passed
    // into tx_start
    //
    uint32_t tx_pnblks;
};

struct tx_manager {
    ods_t *tm_ods;
    lock_t *tm_lock;
    tx_t *tm_tx;
    cv_t *tm_fcv; // flush cv
};

int tx_start(tx_mgr_t *tm, uint32_t nblocks, tid_t *tid);
int tx_finish(tx_mgr_t *tm, tid_t tid);
int tx_flush(tx_mgr_t *tm);

void tx_add_block(tx_mgr_t *tm, blk_t *blk, tid_t tid);

uint32_t tx_state(tx_mgr_t *tm);
uint32_t tx_state_locked(tx_mgr_t *tm);

bool tx_contains_block(tx_mgr_t *tm, blk_t *blk);
bool tx_contains_block_locked(tx_mgr_t *tm, blk_t *blk);

int tx_mgr_startup(ods_t *ods);
int tx_mgr_shutdown(tx_mgr_t *tm);

void tx_mgr_dump(tx_mgr_t *tm);
void tx_mgr_check(tx_mgr_t *tm);

#endif // _TRANSACTION_H_
