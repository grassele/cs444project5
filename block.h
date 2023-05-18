#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SIZE 4096
#define FREE_INODE_MAP_BLOCK_NUM 1
#define FREE_BLOCK_MAP_BLOCK_NUM 2

unsigned char *bread(int block_num, unsigned char *block);
void bwrite(int block_num, unsigned char *block);
int alloc(void);

#endif