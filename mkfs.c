#include "mkfs.h"

/* Create the file system, in an open image */

void mkfs(void) {
    // (1)  Write 1024 blocks of all zero bytes, sequentially, using the write() syscall. 
        /// This will make a 4096*1024-byte image, or 4 MB.

    // (2) Mark data blocks 0-6 as allocated by calling alloc() 7 times.

    // (3) (not part of project 5) Add the root directory and other things needed to bootstrap the file system.
}