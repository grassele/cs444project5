#include "free.h"
#include "block.h"   // for BLOCK_SIZE macro (thought it would be better than defining it separately)


/* Finds the lowest clear (not-set, zero) bit in a byte */

int find_low_clear_bit(unsigned char x){
    for (int i = 0; i < 8; i++)
        if (!(x & (1 << i)))
            return i;
    return -1;
}



/* Set a specific bit to the value in "set" (0 or 1) */

void set_free(unsigned char *block, int num, int set) {   // not sure if we want void to be the return type
    
    int byte_num = num / 8; // truncates automatically (integer division)
    int bit_num = num % 8;

    if (set == 0) {
        block[byte_num] &= ~(!set << bit_num);
    } 
    else {
        block[byte_num] |= set << bit_num;
    }
}



/* Find a 0 bit in the bitmap of the block specified and return its index -- pass in block 1 for 
free inodes or 2 for free data blocks */

int find_free(unsigned char *block) {

    for (int i = 0; i < BLOCK_SIZE; i++) {  // do we need to go through all 4096 bytes? or we don't for inode map but do for block?
        int bit_num = find_low_clear_bit(block[i]);
        if (bit_num != -1) {
            return (i * BLOCK_SIZE) + bit_num;
        }
    }
    return -1;
}