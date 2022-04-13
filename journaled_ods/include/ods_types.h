#ifndef _ODS_TYPES_H_
#define _ODS_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

// ods
typedef struct on_disk_system ods_t;

// blocks
typedef struct block blk_t;
typedef struct blk_phys blk_phys_t;
typedef struct block_list blk_list_t;
typedef struct block_tailq blk_tailq_t;

// transactions
typedef struct tx tx_t;
typedef struct tx_manager tx_mgr_t;
typedef uint64_t tid_t;

// block cache
typedef struct bcache bcache_t;
typedef struct bco_ops bco_ops_t;

// journal
typedef struct journal jnl_t;
typedef struct jnl_blk jnl_blk_t;
typedef struct jnl_blk_phys jnl_blk_phys_t;
typedef struct jnl_super_phys jnl_super_phys_t;
typedef struct jnl_txd_phys jnl_txd_phys_t;
typedef struct jnl_txc_phys jnl_txc_phys_t;

// ods blocks
typedef struct ods_block ods_blk_t;
typedef struct ods_block_phys ods_blk_phys_t;

#endif // _ODS_TYPES_H_
