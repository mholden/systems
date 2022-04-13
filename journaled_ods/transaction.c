#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "transaction.h"
#include "on_disk_system.h"
#include "journal.h"

int tx_start(tx_mgr_t *tm, uint32_t nblocks, tid_t *tid) {
    ods_t *ods;
    tx_t *tx;
    int err;
    
    lock_lock(tm->tm_lock);
    
    // TODO: deal with nblocks being too large

    ods = tm->tm_ods;
    tx = tm->tm_tx;
    
    if (tx->tx_state == TX_IDLE) { // start a new transaction
        memset(tx, 0, sizeof(tx_t));
        tx->tx_id = jnl_get_next_tid(ods->ods_jnl);
        tx->tx_state = TX_OPEN;
        assert(LIST_EMPTY(&tx->tx_blks));
    } else { // join an open transaction
        assert(tx->tx_state == TX_OPEN);
    }
    
    tx->tx_refs++;
    *tid = tx->tx_id;
    
    lock_unlock(tm->tm_lock);
    
    return 0;
    
error_out:
    lock_unlock(tm->tm_lock);
    
    return err;
}

void tx_add_block(tx_mgr_t *tm, blk_t *b, tid_t tid) {
    tx_t *tx;
    lock_lock(tm->tm_lock);
    tx = tm->tm_tx;
    LIST_INSERT_HEAD(&tx->tx_blks, b, bl_tx_link);
    tx->tx_nblks++;
    assert(tid == tx->tx_id);
    b->bl_tid = tid;
    lock_unlock(tm->tm_lock);
}

// called with tm_lock locked
static int tx_flush_locked(tx_mgr_t *tm) {
    ods_t *ods = tm->tm_ods;
    jnl_t *jnl = ods->ods_jnl;
    tx_t *tx = tm->tm_tx;
    tid_t tid = tx->tx_id;
    blk_t *b, *bnext;
    int err;
    
    err = jnl_write_transaction(jnl, &tx->tx_blks, tx->tx_nblks, tid);
    if (err)
        goto error_out;
    
    // we consider the blocks clean now that they're safely in the journal
    LIST_FOREACH_SAFE(b, &tx->tx_blks, bl_tx_link, bnext) {
        assert(b->bl_flags & B_DIRTY);
        b->bl_flags &= ~(B_DIRTY);
        LIST_REMOVE(b, bl_dl_link);
        
        b->bl_tid = TX_ID_NONE;
        LIST_REMOVE(b, bl_tx_link);
        
        tx->tx_nblks--;
    }
    assert(LIST_EMPTY(&tx->tx_blks) && (tx->tx_nblks == 0));
    
    return 0;
    
error_out:
    return err;
}

// flushes the transaction (/ all dirty blocks) to the journal
int tx_flush(tx_mgr_t *tm) {
    tx_t *tx;
    int err;
    
    lock_lock(tm->tm_lock);
    
    tx = tm->tm_tx;
    if (tx->tx_state != TX_OPEN) // nothing to do
        goto out;
    
    err = tx_flush_locked(tm);
    if (err)
        goto error_out;
    
    tx->tx_state = TX_IDLE;
    tx->tx_id = TX_ID_NONE;
    
out:
    lock_unlock(tm->tm_lock);
    
    return 0;
    
error_out:
    lock_unlock(tm->tm_lock);
    
    return err;
}

int tx_finish(tx_mgr_t *tm, tid_t tid) {
    tx_t *tx;
    int err;
    
    lock_lock(tm->tm_lock);
    
    tx = tm->tm_tx;
    
    if (tx->tx_state != TX_OPEN) {
        err = EINVAL;
        goto error_out;
    }
    
    if (tid != tx->tx_id) {
        err = EINVAL;
        goto error_out;
    }
    
    assert(tx->tx_refs > 0);
    tx->tx_refs--;
    
    if (tx->tx_refs == 0) { // no other threads in the transaction. push it out to the journal
        err = tx_flush_locked(tm);
        if (err)
            goto error_out;
        
        tx->tx_state = TX_IDLE;
        tx->tx_id = TX_ID_NONE;
    }
    
    lock_unlock(tm->tm_lock);
    
    return 0;
    
error_out:
    lock_unlock(tm->tm_lock);
    
    return err;
}

bool tx_is_open_locked(tx_mgr_t *tm) {
    tx_t *tx = tm->tm_tx;
    return (tx->tx_state == TX_OPEN);
}

bool tx_is_open(tx_mgr_t *tm) {
    bool open;
    
    lock_lock(tm->tm_lock);
    open = tx_is_open_locked(tm);
    lock_unlock(tm->tm_lock);
    
    return open;
}

bool tx_contains_block_locked(tx_mgr_t *tm, blk_t *b) {
    tx_t *tx;
    blk_t *_b;
    bool contains_block = false;
    
    tx = tm->tm_tx;
    if (!LIST_EMPTY(&tx->tx_blks)) {
        LIST_FOREACH(_b, &tx->tx_blks, bl_tx_link) {
            if (_b == b) {
                contains_block = true;
                break;
            }
        }
    }
    
    return contains_block;
}

bool tx_contains_block(tx_mgr_t *tm, blk_t *b) {
    bool contains_block;
    lock_lock(tm->tm_lock);
    contains_block = tx_contains_block_locked(tm, b);
    lock_unlock(tm->tm_lock);
    return contains_block;
}

int tx_mgr_startup(ods_t *ods) {
    tx_mgr_t *tm;
    int err;
    
    tm = malloc(sizeof(tx_mgr_t));
    if (!tm) {
        err = ENOMEM;
        goto error_out;
    }
    
    memset(tm, 0, sizeof(tx_mgr_t));
    tm->tm_ods = ods;
    
    tm->tm_lock = lock_create();
    if (!tm->tm_lock) {
        err = ENOMEM;
        goto error_out;
    }
    
    tm->tm_tx = malloc(sizeof(tx_t));
    if (!tm->tm_tx) {
        err = ENOMEM;
        goto error_out;
    }
    
    memset(tm->tm_tx, 0, sizeof(tx_t));
    tm->tm_tx->tx_id = TX_ID_NONE;

    ods->ods_tm = tm;
    
    return 0;
    
error_out:
    if (tm) {
        if (tm->tm_lock)
            lock_destroy(tm->tm_lock);
        free(tm);
    }
    return err;
}

int tx_mgr_shutdown(tx_mgr_t *tm) {
    ods_t *ods;
    tx_t *tx;
    int err;
    
    ods = tm->tm_ods;
    tx = tm->tm_tx;
    
    assert(tx->tx_state == TX_IDLE);
    
    err = lock_destroy(tm->tm_lock);
    if (err)
        goto error_out;
    
    free(tm->tm_tx);
    free(tm);
    ods->ods_tm = NULL;
    
    return 0;
    
error_out:
    return err;
}

static const char *tx_state_to_string(uint32_t state) {
    switch (state) {
        case TX_IDLE:
            return "TX_IDLE";
        case TX_OPEN:
            return "TX_OPEN";
        default:
            return "UNKNOWN";
    }
}

// appropriate locks must be held
void tx_mgr_dump(tx_mgr_t *tm) {
    tx_t *tx = tm->tm_tx;
    blk_t *b;
    
    printf("tm @ %p: tx_id 0x%" PRIx64 " tx_refs %" PRIu32 " tx_state %s tx_nblks %" PRIu32 " tx_blks:\n",
            tm, tx->tx_id, tx->tx_refs, tx_state_to_string(tx->tx_state), tx->tx_nblks);
    LIST_FOREACH(b, &tx->tx_blks, bl_tx_link) {
        printf(" ");
        bl_dump(b);
        printf("\n");
    }
}

// appropriate locks must be held
void tx_mgr_check(tx_mgr_t *tm) {
    tx_t *tx;
    blk_t *b;
    uint32_t ntxblocks = 0;
    
    tx = tm->tm_tx;
    assert((tx->tx_state == TX_IDLE) || (tx->tx_state == TX_OPEN));
    if (tx->tx_state == TX_IDLE) {
        assert(tx->tx_id == TX_ID_NONE);
        assert(tx->tx_refs == 0);
        assert(LIST_EMPTY(&tx->tx_blks));
        assert(tx->tx_nblks == 0);
    } else { // TX_OPEN
        assert(tx->tx_id != TX_ID_NONE);
        assert(tx->tx_refs != 0);
        LIST_FOREACH(b, &tx->tx_blks, bl_tx_link) {
            assert(b->bl_flags & B_DIRTY);
            ntxblocks++;
        }
        assert(tx->tx_nblks == ntxblocks);
    }
}
