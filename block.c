#include <unistd.h>  // lseek()
#include "block.h"
#include "free.h"
#include "image.h"



/* Load data from designated block into the buffer */

unsigned char *bread(int block_num, unsigned char *block) {

    size_t block_offset = block_num * BLOCK_SIZE;
    lseek(image_fd, block_offset, SEEK_SET);  // SEEK_SET offsets from file beginning
    read(image_fd, block, BLOCK_SIZE); // reads the whole block, do we want to just do part of it? the size of what's stored?

    return block;
}



/* Write data from buffer into specified block */

void bwrite(int block_num, unsigned char *block) {

    size_t block_offset = block_num * BLOCK_SIZE;
    lseek(image_fd, block_offset, SEEK_SET);
    write(image_fd, block, BLOCK_SIZE);
    
    // should there be a return here?
}



/* Allocate a previous-free data block from the block map */

int alloc(void) {
    // find the lowest free inode in the bit map, mark it allocated, and return number or -1 if no free inodes

    // get free block map w bread() -- block 2 is free block bit map
    unsigned char bbit_map[BLOCK_SIZE];
    strncpy(bbit_map, bread(FREE_BLOCK_MAP_BLOCK_NUM, bbit_map), BLOCK_SIZE);

    // find_free() to locate a free block
    int bnum = find_free(bbit_map);
    if (bnum == -1) {  // no free block found in bit map
        return -1;
    }
    else {
        // set_free() to mark it as non-free
        set_free(bbit_map, bnum, 1);
        // bwrite() to save the block back out to disk - save whole block or just bit map section?
        bwrite(FREE_BLOCK_MAP_BLOCK_NUM, bbit_map);
        return bnum;
    }
}