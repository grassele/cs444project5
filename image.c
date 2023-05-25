#include <fcntl.h>   // open()
#include <unistd.h>  // close()
#include <stdio.h>    // perror()
#include "image.h"

int image_fd;



/* Opens the image file of the given name, creating it if it doesn't exist, and truncating it to 0 size 
if truncate is true */ 

int image_open(char *filename, int truncate) {

    int flags = O_RDWR | O_CREAT;
    if (truncate) 
        flags |= O_TRUNC;

    int fd = open(filename, flags, 0600);

    if (fd == -1) {
        perror("Failed to open image file\n");
        return -1;
    }    
    else {
        image_fd = fd;
        return image_fd;
    }
}



/* Closes the image file using the global image file descriptor */

int image_close(void) {
    return close(image_fd);
}