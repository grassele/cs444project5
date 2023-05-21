#include "inode.h"
#include "block.h"
#include "free.h"

/* Allocate a previously-free inode in the inode map */

int ialloc(void) {
    // Find the lowest free inode in the bit map, mark it allocated, and return number or -1 if no free inodes

    // Get inode map
    unsigned char ibit_map[BLOCK_SIZE];
    ibit_map = bread(FREE_INODE_MAP_BLOCK_NUM, ibit_map);

    // Locate a free inode
    int inum = find_free(ibit_map);
    if (inum == -1) {  // no free inode found in bit map
        return -1;
    }
    else {
        // Mark block bit as non-free
        set_free(ibit_map, inum, 1);
        //Save the inode back out to disk - save inode or bit map?
        bwrite(FREE_INODE_MAP_BLOCK_NUM, ibit_map);
    }
}