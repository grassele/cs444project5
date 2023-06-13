#include "inode.h"
#include "block.h"
#include "free.h"
#include "pack.h"
#include "image.h"
#include <unistd.h>
#include <string.h>   // strncpy()
#include <stdio.h>    // included for printf for testing purposes



static struct inode incore[MAX_SYS_OPEN_FILES] = {0};



/* Reset the ref_count of every incore inode to 0 */

void reinitialize_incore(void) {
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        incore[i].ref_count = 0;
    }
}



/* Allocate a previously-free inode in the inode map */

int ialloc(void) {

    // Find the lowest free inode in the bit map, mark it allocated, and return number or -1 if no free inodes

    // Get inode map
    unsigned char ibit_map[BLOCK_SIZE];
    bread(FREE_INODE_MAP_BLOCK_NUM, ibit_map);

    // Locate a free inode
    int inum = find_free(ibit_map);
    if (inum == -1) {  // no free inode found in bit map
        return -1;
    }
    else {
        // Mark block bit as non-free
        set_free(ibit_map, inum, 1);
        // Save the inode back out to disk - save inode or bit map?
        bwrite(FREE_INODE_MAP_BLOCK_NUM, ibit_map);
        return inum;
    }
}



/* Finds the first inode in the in-core inode array, returning a pointer or NULL if no free inodes */

struct inode *find_incore_free(void) {

    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count == 0) {
            return &incore[i];  // or should this just be incore[i]
        }
    }
    return NULL;  // no free inodes
}



/* Finds an in-core inode within the array matching the given inode_num, returning pointer to it or NULL if not found */

struct inode *find_incore(unsigned int inode_num) {

    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count != 0 && incore[i].inode_num == inode_num) {
            return &incore[i];
        }
    }
    return NULL;  // the particular inode was not found in the incore inode array
}



/* Reads data from inode w inode_num from disk into buffer */

void read_inode(struct inode *in, int inode_num) {

    int block_num = (inode_num / INODES_PER_BLOCK) + INODE_FIRST_BLOCK;   // integer division to find how far into the inode blocks it is
    int block_offset = inode_num % INODES_PER_BLOCK;   // offset in inodes

    // location offset from beginning of file system block
    int total_offset_bytes = (block_num * BLOCK_SIZE) + (block_offset * INODE_SIZE);
    unsigned char block[INODE_SIZE];
    lseek(image_fd, total_offset_bytes, SEEK_SET);
    read(image_fd, block, INODE_SIZE);

    // go through block, reading one little section at a time, and writing into the fields of the struct

    // in an inode, the size is stored at byte 0 and is 4 bytes long
    void *file_size = (void *)&block[0];
    in->size = read_u32(file_size);

    // owner_id is at byte 4 and is 2 bytes long
    void *owner_id = (void *)&block[4];
    in->owner_id = read_u16(owner_id);

    // permissions is at byte 6 and is 1 byte long
    void *permissions = (void *)&block[6];
    in->permissions = read_u8(permissions);

    // flags is at byte 7 and is 1 byte long
    void *flags = (void *)&block[7];
    in->flags = read_u8(flags);

    // link_count is at byte 8 and is 1 byte long
    void *link_count = (void *)&block[8];
    in->link_count = read_u8(link_count);

    // the block_ptr array is at byte 9 and is 32 bytes long (INODE_PTR_COUNT = 16, 1 * 2 = 32)
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16((void *)&block[9+(2*i)]);  // block[9] is the location of the beginning of the array of block pointers
    }

    // initialize ref_count to 1
    in->ref_count = 0;

    // set inode_num field to inode_num
    in->inode_num = inode_num;
}



/* Writes data stored in incore inode back to disk inode */

void write_inode(struct inode *in) {

    int inode_num = in->inode_num;
    int block_num = (inode_num / INODES_PER_BLOCK) + INODE_FIRST_BLOCK;   // integer division to find how far into the inode blocks it is
    int block_offset = inode_num % INODES_PER_BLOCK;   // offset in inodes

    // location offset from beginning of file system block
    int total_offset_bytes = (block_num * BLOCK_SIZE) + (block_offset * INODE_SIZE);
    unsigned char block[INODE_SIZE];

    // go through incore inode, reading one field at a time, and writing it into the block

    // in an inode, the size is stored at byte 0 and is 4 bytes long
    write_u32(&block[0], in->size);

    // owner_id is at byte 4 and is 2 bytes long
    write_u16(&block[4], in->owner_id);

    // permissions is at byte 6 and is 1 byte long
    write_u8(&block[6], in->permissions);

    // flags is at byte 7 and is 1 byte long
    write_u8(&block[7], in->flags);

    // link_count is at byte 8 and is 1 byte long
    write_u8(&block[8], in->link_count);

    // the block_ptr array is at byte 9 and is 32 bytes long (INODE_PTR_COUNT = 16, 16 * 2 = 32)
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(&block[9+(2*i)], in->block_ptr[i]);
    }

    lseek(image_fd, total_offset_bytes, SEEK_SET);
    write(image_fd, block, INODE_SIZE);
}
