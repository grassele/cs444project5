#define CTEST_ENABLE
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "mkfs.h"
#include <fcntl.h>
#include <string.h>   // memcmp()


/////  image.c tests  /////////////////////////////////////////////////////////////////////////////


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


/////  block.c tests  /////////////////////////////////////////////////////////////////////////////


void test_bread_and_bwrite(void) {

    image_open("new_image", 1);
    unsigned char test_block_write_in[] = "test_block_contents";
    int test_block_num = 13;
    bwrite(test_block_num, test_block_write_in);
    unsigned char test_block_read_out[BLOCK_SIZE];
    bread(test_block_num, test_block_read_out);
    CTEST_ASSERT(memcmp(test_block_write_in, test_block_read_out, BLOCK_SIZE) == 0, "bwrite successfully writes and bread successfully reads");
}

/* similar functionality to ialloc in inode.c; updates should be to both functions */

void test_alloc(void) {
    ;
}


/////  free.c tests  //////////////////////////////////////////////////////////////////////////////


void test_set_and_find_free(void) {

    unsigned char *block_for_block_map[BLOCK_SIZE];
    bread(FREE_BLOCK_MAP_BLOCK_NUM, (unsigned char *)block_for_block_map);
    CTEST_ASSERT(find_free((unsigned char *)block_for_block_map) == 0, "first free block before any setting is block 0");

    set_free((unsigned char *)block_for_block_map, 0, 1);
    CTEST_ASSERT(find_free((unsigned char *)block_for_block_map) == 1, "first free block after setting 0 block is 1");

    unsigned char *fake_full_bit_map[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE * 8; i++) {
        set_free((unsigned char *)fake_full_bit_map, i, 1);
    }
    CTEST_ASSERT(find_free((unsigned char *)fake_full_bit_map) == -1, "returns -1 when bit map is full");
}


/////  inode.c tests  /////////////////////////////////////////////////////////////////////////////


/* similar functionality to alloc in block.c; updates should be to both functions */

void test_ialloc(void) {

    // do we need to reset anything here so that the tests won't break?

    // returns num of the newly allocated inode (num >= 0)
    // find_free now returns greater or equal to 1
    // returns -1 if all inodes are full
}


/////  mkfs.c tests  //////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////


/* Assuming that all test functions have been run and all need their leftover image files deleted */

void delete_test_image_files(void) {

    remove("image1");
    remove("image2");
    remove("image_to_close");
    remove("new_image");
}


int main(void) {

    CTEST_VERBOSE(1);

    test_image_open();
    test_image_close();
    test_bread_and_bwrite();
    test_set_and_find_free();
    test_ialloc();

    delete_test_image_files();

    CTEST_RESULTS();

    CTEST_EXIT();

}