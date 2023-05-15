#include "image.h"


int main(void) {
    image_open("diskimage.dat", 1);
    // use mkfs() to create a file system in the open image
    image_close();
}