#ifndef MKFS_H
#define MKFS_H

#define NUM_BLOCKS 1024
#define NUM_PRECLAIMED_BLOCKS 7
#define DIRECTORY_ENTRY_SIZE 32



struct directory {
    struct inode *inode;
    unsigned int offset;
};

struct directory_entry {
    unsigned int inode_num;
    char name[16];
};



struct directory *directory_open(int inode_num);

int directory_get(struct directory *dir, struct directory_entry *ent);

void directory_close(struct directory *d);

void mkfs(void);



#endif