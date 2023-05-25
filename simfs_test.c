#define CTEST_ENABLE
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "mkfs.h"
#include <fcntl.h>


void test_image_open(void) {

    int image_fd1 = image_open("image1", 1);
    CTEST_ASSERT(image_fd1 != -1, "opening a single image does not fail");

    int image_fd2 = image_open("image2", 1);
    CTEST_ASSERT(image_fd2 != image_fd1, "opening a second image does not have the same file descriptor");

    int image_fd3 = image_open("/image3", 1);
    CTEST_ASSERT(image_fd3 == -1, "creating a file in the root directory");
    
}


void test_image_close(void) {

    image_fd = image_open("image_to_close", 1);
    int fd_state_before_close = fcntl(image_fd, F_GETFD);
    image_close();
    int fd_state_after_close = fcntl(image_fd, F_GETFD);
    CTEST_ASSERT(fd_state_before_close == 0 && fd_state_after_close == -1, "closing an image makes the old fd invalid");

    image_fd = -1; // invalid file descriptor
    CTEST_ASSERT(image_close() == -1, "closing an image file with file descriptor -1 fails");

}






int main(void) {

    CTEST_VERBOSE(1);

    test_image_open();
    test_image_close();

    CTEST_RESULTS();

    CTEST_EXIT();

}