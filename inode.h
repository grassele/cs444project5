#ifndef INODE_H
#define INODE_H

#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

#define NUM_INODE_BLOCKS 4
#define MAX_NUM_INODES (NUM_INODE_BLOCKS * INODES_PER_BLOCK)

#define INODE_PTR_COUNT 16

#define MAX_SYS_OPEN_FILES 64

#define UNINCLUSIVE_MAX_INT_W_4_BYTES 4294967296


struct inode {
    unsigned int size;
    unsigned short owner_id;
    unsigned char permissions;
    unsigned char flags;
    unsigned char link_count;
    unsigned short block_ptr[INODE_PTR_COUNT];

    unsigned int ref_count;  // in-core only
    unsigned int inode_num;
};

void reinitialize_incore(void);  // added for project 6

struct inode *ialloc(void);

struct inode *find_incore_free(void);

struct inode *find_incore(unsigned int inode_num);

void read_inode(struct inode *in, int inode_num);

void write_inode(struct inode *in);

struct inode *iget(int inode_num);

void iput(struct inode *in);

#endif