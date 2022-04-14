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
    
    ods = tm->tm_ods;
    tx = tm->tm_tx;
    
try_again:
    if (tx->tx_state == TX_FLUSHING) { // wait for flush to finish
        while (tx->tx_state == TX_FLUSHING) {
            cv_wait(tm->tm_fcv, tm->tm_lock);
        }
    }
    
    if (nblocks > jnl_capacity(ods->ods_jnl)) {
        err = E2BIG;
        goto error_out;
    }
    
    if ((tx->tx_pnblks + nblocks) > jnl_capacity(ods->ods_jnl)) {
        //
        // this transcation may bring us over the max allowable
        // transaction size, so we can't join it. tx_flush and
        // then try again
        //
        lock_unlock(tm->tm_lock);
        err = tx_flush(tm);
        if (err)
            goto error_out;
        lock_lock(tm->tm_lock);
        goto try_again;
    }
    
    if (tx->tx_state == TX_IDLE) { // start a new transaction
        tx->tx_id = jnl_get_next_tid(ods->ods_jnl);
        tx->tx_state = TX_OPEN;
        assert(LIST_EMPTY(&tx->tx_blks) && (tx->tx_nblks == 0));
    } else { // join an open transaction
        assert(tx->tx_state == TX_OPEN);
    }
    
    tx->tx_pnblks += nblocks;
    tx->tx_refs++;
    *tid = tx->tx_id;
    
    lock_unlock(tm->tm_lock);
    
    return 0;
    
error_out:
    lock_unlock(tm->tm_lock);
    
    return err;
}

//
// this function is called without the tm lock held. not holding the
// tm lock allows us to take the bc lock within this function to update
// dirty / free lists as necessary. we synchronize this with the TX_FLUSHING
// flag instead of the tm lock.
//
static int _tx_flush(tx_mgr_t *tm) {
    ods_t *ods = tm->tm_ods;
    jnl_t *jnl = ods->ods_jnl;
    bcache_t *bc = ods->ods_bc;
    tx_t *tx = tm->tm_tx;
    tid_t tid = tx->tx_id;
    blk_t *b, *bnext;
    int err;
    
    assert((tx->tx_state == TX_FLUSHING) && (tx->tx_refs == 0));
    
    err = jnl_write_transaction(jnl, &tx->tx_blks, tx->tx_nblks, tid);
    if (err)
        goto error_out;
    
    //
    // we consider the blocks clean now that they're safely in the
    // journal. it's safe to take the bc lock here because we don't
    // hold the tm lock while flushing
    //
    lock_lock(bc->bc_lock);
    LIST_FOREACH_SAFE(b, &tx->tx_blks, bl_tx_link, bnext) {
        assert(b->bl_flags & B_DIRTY);
        b->bl_flags &= ~(B_DIRTY);
        LIST_REMOVE(b, bl_dl_link);
        bc->bc_ndirty--;
        
        if (b->bl_refcnt == 0) {
            TAILQ_INSERT_TAIL(&bc->bc_fl, b, bl_fl_link);
            bc->bc_nfree++;
        }
        
        b->bl_tid = TX_ID_NONE;
        LIST_REMOVE(b, bl_tx_link);
        
        tx->tx_nblks--;
    }
    assert(LIST_EMPTY(&bc->bc_dl) && (bc->bc_ndirty == 0));
    lock_unlock(bc->bc_lock);
    assert(LIST_EMPTY(&tx->tx_blks) && (tx->tx_nblks == 0));
    
    tx->tx_pnblks = 0;
    tx->tx_id = TX_ID_NONE;
    tx->tx_state = TX_IDLE;
    
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
    
    if (tx->tx_state == TX_FLUSHING) {
        //
        // another thread is already flushing.
        // just wait for the flush to finish
        //
        while (tx->tx_state == TX_FLUSHING) {
            cv_wait(tm->tm_fcv, tm->tm_lock);
        }
        goto locked_out;
    }
    
    if (tx->tx_state == TX_IDLE) // nothing to do
        goto locked_out;
    
    assert(tx->tx_state == TX_OPEN);
    if (tx->tx_nblks == 0) { // nothing to do
        assert(LIST_EMPTY(&tx->tx_blks));
        goto locked_out;
    }
    
    tx->tx_state = TX_FLUSHING;
    
    // we need to wait until everyone is out of the transaction
    while (tx->tx_refs != 0)
        cv_wait(tm->tm_fcv, tm->tm_lock);
    
    lock_unlock(tm->tm_lock);
    
    err = _tx_flush(tm);
    if (err) {
        assert(tx->tx_state == TX_FLUSHING); // should still be set to flushing on error
        tx->tx_state = TX_OPEN; // move back to TX_OPEN
        cv_broadcast(tm->tm_fcv); // and tell blocked threads we're not flushing anymore
        goto error_out;
    }
    
    cv_broadcast(tm->tm_fcv);
    
    return 0;
    
locked_out:
    lock_unlock(tm->tm_lock);
    
    return 0;
    
error_out:
    return err;
}

int tx_finish(tx_mgr_t *tm, tid_t tid) {
    ods_t *ods;
    tx_t *tx;
    bool do_flush = false;
    int err;
    
    lock_lock(tm->tm_lock);
    
    ods = tm->tm_ods;
    tx = tm->tm_tx;
    
    assert(!(tx->tx_state == TX_IDLE));
    assert(tid == tx->tx_id);
    
    assert(tx->tx_refs > 0);
    tx->tx_refs--;
    
    if (tx->tx_refs == 0) {
        cv_broadcast(tm->tm_fcv); // wake up anyone waiting to flush
        
        // trigger a flush if the transaction is getting too large
        if ((tx->tx_state != TX_FLUSHING) &&
                (tx->tx_pnblks > (jnl_capacity(ods->ods_jnl) * 3 / 4)))
            do_flush = true;
    }
    
    lock_unlock(tm->tm_lock);
    
    if (do_flush) {
        err = tx_flush(tm);
        if (err)
            goto error_out;
    }
    
    return 0;
    
error_out:
    lock_unlock(tm->tm_lock);
    
    return err;
}

void tx_add_block(tx_mgr_t *tm, blk_t *b, tid_t tid) {
    ods_t *ods;
    tx_t *tx;
    lock_lock(tm->tm_lock);
    ods = tm->tm_ods;
    tx = tm->tm_tx;
    assert(tx->tx_refs); // guarantees we're not in _tx_flush
    LIST_INSERT_HEAD(&tx->tx_blks, b, bl_tx_link);
    tx->tx_nblks++;
    assert(tx->tx_nblks <= tx->tx_pnblks);
    assert(tid == tx->tx_id);
    b->bl_tid = tid;
    lock_unlock(tm->tm_lock);
}

uint32_t tx_state_locked(tx_mgr_t *tm) {
    tx_t *tx = tm->tm_tx;
    return tx->tx_state;
}

uint32_t tx_state(tx_mgr_t *tm) {
    uint32_t state;
    
    lock_lock(tm->tm_lock);
    state = tx_state_locked(tm);
    lock_unlock(tm->tm_lock);
    
    return state;
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
    
    tm->tm_fcv = cv_create();
    if (!tm->tm_fcv) {
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
        if (tm->tm_fcv)
            cv_destroy(tm->tm_fcv);
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
    
    err = cv_destroy(tm->tm_fcv);
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
        case TX_FLUSHING:
            return "TX_FLUSHING";
        default:
            return "UNKNOWN";
    }
}

// appropriate locks must be held
void tx_mgr_dump(tx_mgr_t *tm) {
    tx_t *tx = tm->tm_tx;
    blk_t *b;
    
    printf("tm @ %p: tx_id 0x%" PRIx64 " tx_refs %" PRIu32 " tx_state %s tx_nblks %" PRIu32 " tx_pnblks %" PRIu32 " tx_blks:\n",
            tm, tx->tx_id, tx->tx_refs, tx_state_to_string(tx->tx_state), tx->tx_nblks, tx->tx_pnblks);
    LIST_FOREACH(b, &tx->tx_blks, bl_tx_link) {
        printf(" ");
        bl_dump(b);
        printf("\n");
    }
}

// appropriate locks must be held
void tx_mgr_check(tx_mgr_t *tm) {
    ods_t *ods;
    tx_t *tx;
    blk_t *b;
    uint32_t ntxblocks = 0;
    
    ods = tm->tm_ods;
    tx = tm->tm_tx;
    assert((tx->tx_state == TX_IDLE) || (tx->tx_state == TX_OPEN) || (tx->tx_state == TX_FLUSHING));
    if (tx->tx_state == TX_IDLE) {
        assert(tx->tx_id == TX_ID_NONE);
        assert(tx->tx_refs == 0);
        assert(LIST_EMPTY(&tx->tx_blks));
        assert(tx->tx_nblks == 0);
        assert(tx->tx_pnblks == 0);
    } else if (tx->tx_state == TX_OPEN) {
        assert(tx->tx_id != TX_ID_NONE);
        LIST_FOREACH(b, &tx->tx_blks, bl_tx_link) {
            assert(b->bl_flags & B_DIRTY);
            ntxblocks++;
        }
        assert(tx->tx_nblks == ntxblocks);
    }
    assert(tx->tx_nblks <= tx->tx_pnblks);
    assert(tx->tx_pnblks <= jnl_capacity(ods->ods_jnl));
}
