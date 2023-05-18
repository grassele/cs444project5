#include <fcntl.h>  // open()
#include <unistd.h>  // close()
#include "image.h"



/* Opens the image file of the given name, creating it if it doesn't exist, and truncating it to 0 size 
if truncate is true */ 

int image_open(char *filename, int truncate) {

    if (truncate) {
        int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600); // 0600 means rw- --- ---
        if (fd == -1) {
            perror("Failed to open image file\n");
        }    
        else {
            image_fd = fd;
            return image_fd;
        }
    }
    else {
        int fd = open(filename, O_RDWR | O_CREAT, 0600);
        if (fd == -1) {
            perror("Failed to open image file\n");
        }    
        else {
            image_fd = fd;
            return image_fd;
        }
    }
}



/* Closes the image file using the global image file descriptor */

int image_close(void) {
    return close(image_fd);
}