#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "block.h"
#include "inode.h"
#include "image.h"
#include "mkfs.h"
#include "pack.h"



/* Open a directory and get a pointer to it to be able to read individual entries */

struct directory *directory_open(int inode_num) {

    struct inode *open_dir_inode = iget(inode_num);

    // return null if iget fails
    if (open_dir_inode == NULL) {
        return NULL;
    }

    struct directory *open_dir = malloc(sizeof(struct directory));
    open_dir->inode = open_dir_inode;
    open_dir->offset = 0;  // we start at the beginning of the directory

    return open_dir;
}



/* Reads the next directory entry into the specified struct directory_entry */

int directory_get(struct directory *dir, struct directory_entry *ent) {

    // if offset within the directory would go off the end of the directory, fail
    if (dir->offset >= dir->inode->size) {
        return -1;
    }

    // find block(s) where directory file is stored
    int data_block_index = dir->offset / BLOCK_SIZE;
    int data_block_num = dir->inode->block_ptr[data_block_index];
    unsigned char directory_data[BLOCK_SIZE];
    bread(data_block_num, directory_data);

    // find the current entry within the directory
    int offset_into_block = dir->offset % BLOCK_SIZE;
    ent->inode_num = read_u16(directory_data + offset_into_block + 0);
    strcpy(ent->name, (char *)directory_data + offset_into_block + 2);

    // update the offset
    dir->offset += DIRECTORY_ENTRY_SIZE;

    return 0;
}



/* When done with the directory, we close it by freeing up the incore inode and the struct directory */

void directory_close(struct directory *d) {

    iput(d->inode);
    free(d);
}



/* Create the file system, in open image */

void mkfs(void) {

    // (1)  Write 1024 blocks of all zero bytes, sequentially, using the write() syscall. 
        /// This will make a 4096*1024-byte image, or 4 MB.

    unsigned char block[BLOCK_SIZE] = {0};

    for (int i = 0; i < NUM_BLOCKS; i++) {
        write(image_fd, block, BLOCK_SIZE);
    }

    // (2) Mark data blocks 0-6 as allocated by calling alloc() 7 times.

    for (int i = 0; i < NUM_PRECLAIMED_BLOCKS; i++){
        alloc();
    }

    // (3) Add the root directory
    
    struct inode *root_directory_inode = ialloc();
    int root_directory_block_num = alloc();
    root_directory_inode->flags = 2;  // 2 means file type is a directory
    root_directory_inode->size = 2 * DIRECTORY_ENTRY_SIZE;  // 2 default entries . and ..
    root_directory_inode->block_ptr[0] = root_directory_block_num;
    unsigned char root_directory_data[BLOCK_SIZE];

    // create directory entry for "current directory"
    write_u16(root_directory_data + 0, root_directory_inode->inode_num);
    strcpy((char *)root_directory_data + 2, ".");

    // create directory entry for "parent directory"
    write_u16(root_directory_data + 32, root_directory_inode->inode_num);
    strcpy((char *)root_directory_data + 34, "..");

    // write directory file contents back out to disk and free the incore inode
    bwrite(root_directory_block_num, root_directory_data);
    iput(root_directory_inode);


    // making sure things work
    struct directory *directory1 = directory_open(0);
    struct directory_entry *entry1 = malloc(sizeof(struct directory_entry));  // maybe i can't do this?
    directory_get(directory1, entry1);
    printf("inode: %d\n", entry1->inode_num);
    printf("name: %s\n", entry1->name);
    directory_get(directory1, entry1);
    printf("inode: %d\n", entry1->inode_num);
    printf("name: %s\n", entry1->name);
}