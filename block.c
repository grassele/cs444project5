#include <unistd.h>  // lseek()
#include "block.h"
#include "image.h"



/* Load data from designated block into the buffer */

unsigned char *bread(int block_num, unsigned char *block) {

    size_t block_offset = block_num * 4096;  // 4096 is block size
    unsigned char *offset_to_block = lseek(image_fd, block_offset, SEEK_SET);  // SEEK_SET offsets from file beginning
    read(image_fd, block, 4096); // 4096 reads the whole block, do we want to just do part of it? the size of what's stored?

    return block;
}



/* Write data from buffer into specified block */

void bwrite(int block_num, unsigned char *block) {

    size_t block_offset = block_num * 4096;
    unsigned char *offset_to_block = lseek(image_fd, block_offset, SEEK_SET);
    write(image_fd, block, 4096);
    
    // should there be a return here?
}