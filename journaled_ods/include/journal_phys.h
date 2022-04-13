#ifndef _JOURNAL_PHYS_H_
#define _JOURNAL_PHYS_H_

#include "ods_types.h"
#include "block_phys.h"

// journal block types (jhp_type)
#define JNL_PHYS_TYPE_SUPER          0
#define JNL_PHYS_TYPE_TX_DESC        1
#define JNL_PHYS_TYPE_TX_COMMIT      2

struct
__attribute__((__packed__))
jnl_blk_phys {
    blk_phys_t jhp_blk;
    uint32_t jhp_type;
};

//
// JNL_SUPER:
//

#define JNL_PHYS_MAGIC 0x0D590D59
#define JNL_PHYS_DEFAULT_SIZE 128 // in blocks

// js_flags:
#define JNLP_JSF_BRAND_NEW 0x00000001
#define JNLP_JSF_VALID_FLAGS (JNLP_JSF_BRAND_NEW)

struct
__attribute__((__packed__))
jnl_super_phys {
    jnl_blk_phys_t js_header;
    uint32_t js_magic;
    uint32_t js_flags;
    uint32_t js_size; // in blocks
    uint32_t js_blksz;
    uint64_t js_next_tid;
    uint32_t js_start_index;
    uint32_t js_end_index;
};

//
// JNL_TX_DESC:
//

struct
__attribute__((__packed__))
jnl_txd_phys {
    jnl_blk_phys_t jtd_header;
    uint64_t jtd_tid; // transaction id
    uint32_t jtd_size; // number of blocks in this transaction (not including tx descriptor or tx commit blocks)
    //uint64_t jtd_map[]; // block addresses of each block in the transaction
};

//
// JNL_TX_COMMIT:
//

struct
__attribute__((__packed__))
jnl_txc_phys {
    jnl_blk_phys_t jtc_header;
    uint64_t jtc_tid; // == jtd_tid tells us that the transaction is good
};

uint32_t jb_phys_type(jnl_blk_phys_t *jbp);
void jb_phys_dump(jnl_blk_phys_t *jbp);

#endif // _JOURNAL_PHYS_H_
