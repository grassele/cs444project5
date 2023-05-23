#include "inode.h"
#include "block.h"
#include "free.h"
#include "pack.h"
#include "image.h"
#include <unistd.h>

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};



/* Allocate a previously-free inode in the inode map */

int ialloc(void) {

    // Find the lowest free inode in the bit map, mark it allocated, and return number or -1 if no free inodes

    // Get inode map
    unsigned char ibit_map[BLOCK_SIZE];
    strncpy(ibit_map, bread(FREE_INODE_MAP_BLOCK_NUM, ibit_map), BLOCK_SIZE);

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



/* Finds the first inode in the in-core inode array, returning a pointer or NULL if no free inodes */

struct inode *find_incore_free(void) {

    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count == 0) {
            return &incore[i];
        }
    }
    return 0;   // no free inodes -- 'identifier NULL is undefined' if 'return NULL'
}



/* Finds an in-core inode within the array matching the given inode_num, returning pointer to it or NULL if not found */

struct inode *find_incore(unsigned int inode_num) {

    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count == 0 && incore[i].inode_num == inode_num) {
            return &incore[i];
        }
    }
    return 0;
}



/* Reads data from inode w inode_num from disk into buffer */

void read_inode(struct inode *in, int inode_num) {

    int block_num = (inode_num / INODES_PER_BLOCK) + INODE_FIRST_BLOCK;   // integer division to find how far into the inode blocks it is
    int block_offset = inode_num % INODES_PER_BLOCK;   // offset in inodes
    int block_offset_bytes = block_offset * INODE_SIZE;
    unsigned char block[INODE_SIZE];
    lseek(image_fd, block_offset_bytes, SEEK_SET);
    read(image_fd, block, INODE_SIZE);
    // go through block, reading one little section at a time, and writing into the fields of the struct
}



/*  */

void write_inode(struct inode *in);





//// Read the byte for flags, assuming `block` is the array we read with `bread()`
// int flags = read_u8(block + block_offset_bytes + 7);