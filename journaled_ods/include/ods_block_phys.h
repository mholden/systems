#ifndef _ODS_BLOCK_PHYS_H_
#define _ODS_BLOCK_PHYS_H_

#include "ods_types.h"
#include "block_phys.h"

struct
__attribute__((__packed__))
ods_block_phys {
    blk_phys_t obp_blk;
    uint64_t obp_data;
};

void ob_phys_dump(ods_blk_phys_t *obp);

#endif // _ODS_BLOCK_PHYS_H_
