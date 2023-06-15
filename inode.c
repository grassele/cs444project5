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



/* Allocate a previously-free inode in the inode map, and return a pointer to the inode loaded into incore */

struct inode *ialloc(void) {

    // Get free inode bit map
    unsigned char ibit_map[BLOCK_SIZE];
    bread(FREE_INODE_MAP_BLOCK_NUM, ibit_map);

    // Locate a free inode in the map
    int inode_num = find_free(ibit_map);

    // If no free inode found in bit map, return NULL
    if (inode_num == -1) {
        return NULL;
    }

    // If free inode was found
    else {

        // Mark block bit as non-free
        set_free(ibit_map, inode_num, 1);

        // Save the inode back out to disk - save inode or bit map?
        bwrite(FREE_INODE_MAP_BLOCK_NUM, ibit_map);
    }

    // Find incore inode to initialize inode data
    struct inode *incore_inode = iget(inode_num);

    // If no free incore inode was found, return NULL
    if (incore_inode == NULL) {
        return NULL;
    }

    // If free incore inode was found
    else {

        // Initialize the incore inode
        incore_inode->size = 0;
        incore_inode->owner_id = 0;
        incore_inode->permissions = 0;
        incore_inode->flags = 0;
        for (int i = 0; i < INODE_PTR_COUNT; i++) {
            incore_inode->block_ptr[i] = 0;
        }
        incore_inode->inode_num = inode_num;

        // Write the initialized data back to disk
        write_inode(incore_inode);

        return incore_inode;
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
    in->size = read_u32(block + 0);

    // owner_id is at byte 4 and is 2 bytes long
    in->owner_id = read_u16(block + 4);

    // permissions is at byte 6 and is 1 byte long
    in->permissions = read_u8(block + 6);

    // flags is at byte 7 and is 1 byte long
    in->flags = read_u8(block + 7);

    // link_count is at byte 8 and is 1 byte long
    in->link_count = read_u8(block + 8);

    // the block_ptr array is at byte 9 and is 32 bytes long (INODE_PTR_COUNT = 16, 1 * 2 = 32)
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16(block + 9 + (2*i));  // block[9] is the location of the beginning of the array of block pointers
    }

    // initialize ref_count to 0
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



/* Get an incore inode for a specific disk inode, either by finding it already in the incore array or loading it in from disk */

struct inode *iget(int inode_num) {

    struct inode *found_incore = find_incore(inode_num);
    if (found_incore != NULL) {
        found_incore->ref_count += 1;
        return found_incore;
    }
    else {
        struct inode *new_inode = find_incore_free();

        // if there are no more available incore inodes
        if (new_inode == NULL) {
            return NULL;
        }
        read_inode(new_inode, inode_num);
        new_inode->ref_count = 1;
        return new_inode;
    }
}



/* Allow the current incore inode to be put back on disk if no one else is using it, effectively freeing up space for a needed disk inode */

void iput(struct inode *in) {

    if (in->ref_count == 0) {
        return;
    }
    in->ref_count -= 1;
    if (in->ref_count == 0) {
        write_inode(in);
    }
}