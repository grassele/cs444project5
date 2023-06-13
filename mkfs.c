#include <unistd.h>
#include "block.h"
#include "image.h"
#include "mkfs.h"

#include <stdio.h> // ELIZABETH DELETE THIS

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
        // printf("block num allocated in mkfs function: %d\n", alloc());  // ELIZABETH REMOVE PRINT PART, LEAVE JUST ALLOC
    }

    // (3) (not part of project 5) Add the root directory and other things needed to bootstrap the file system.
}