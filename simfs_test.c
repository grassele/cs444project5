#include "image.h"
#include "block.h"
#include "free.h"
#include "mkfs.h"


void test_image_open(void) {
    int image_fd1 = image_open('image1', 1);
    CTEST_ASSERT(image_fd1 != -1, 'opening a single image does not fail');
    int image_fd2 = image_open('image2', 1);
    CTEST_ASSERT(image_fd2 != image_fd1, 'opening a second image does not have the same file descriptor');
}



int main(void) {

    image_open("diskimage.dat", 1);
    // use mkfs() to create a file system in the open image
    image_close();

    CTEST_VERBOSE(1);

    

    CTEST_RESULTS();

    CTEST_EXIT();

}