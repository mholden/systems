#ifndef _ON_DISK_SYSTEM_PHYS_H_
#define _ON_DISK_SYSTEM_PHYS_H_

//
// our on disk system is simple. block 0 is where the journal lives.
// following the journal (depending on the journal size) are 'ods blocks'
// (ODS_BP_TYPE_BLOCK). ods blocks are grouped together into groups of
// size ODS_BLOCKS_PER_GROUP. all ods blocks in the same group contain
// the same data in a consistent system. the journal allows us to
// maintain this consistency. that's it.
//

#define ODS_JOURNAL_OFFSET      0

// ods block phys types
#define ODS_PHYS_TYPE_JOURNAL     0
#define ODS_PHYS_TYPE_BLOCK       1

#define ODS_BLOCKS_PER_GROUP 10

#endif // _ON_DISK_SYSTEM_PHYS_H_
