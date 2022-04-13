#ifndef _BLOCK_PHYS_H_
#define _BLOCK_PHYS_H_

#include "ods_types.h"

// all on disk blocks start with this header
struct
__attribute__((__packed__))
blk_phys {
    uint32_t bp_type;
    // uint8_t bp_data[];
};

uint32_t bl_phys_type(blk_phys_t *bp);
void bl_phys_dump(blk_phys_t *bp);

#endif // _BLOCK_PHYS_H_
