#define CTEST_ENABLE
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"
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
    CTEST_ASSERT(image_fd3 == -1, "creating a file in the root directory fails");
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

    image_open("test_image", 1);

    unsigned char test_block_write_in[BLOCK_SIZE] = {0};
    strcpy((char*)&test_block_write_in, "test_block_contents");
    int test_block_num = 13;
    bwrite(test_block_num, test_block_write_in);
    unsigned char test_block_read_out[BLOCK_SIZE];
    bread(test_block_num, test_block_read_out);
    CTEST_ASSERT(memcmp(test_block_write_in, test_block_read_out, BLOCK_SIZE) == 0, "bwrite successfully writes and bread successfully reads");
    
    // reset image
    image_close();
}


/* Similar functionality to ialloc in inode.c; updates should be to both functions */

void test_alloc(void) {

    image_open("test_image", 1);
    mkfs();

    unsigned char ta_free_block_bit_map[BLOCK_SIZE];
    bread(FREE_BLOCK_MAP_BLOCK_NUM, ta_free_block_bit_map);
    int first_free_before_alloc = find_free(ta_free_block_bit_map);
    int allocated_block = alloc();
    CTEST_ASSERT(allocated_block >= 0 && allocated_block < NUM_BLOCKS_IN_FILE_SYS, "block num returned is contained in the bit array");

    CTEST_ASSERT(first_free_before_alloc == allocated_block, "block allocated by alloc is the first free block");

    int first_free_after_alloc = find_free(bread(FREE_BLOCK_MAP_BLOCK_NUM, ta_free_block_bit_map));
    CTEST_ASSERT(first_free_after_alloc != first_free_before_alloc, "alloc takes the first free block, making it no longer the first free block");

    unsigned char ta_fake_full_bit_map[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE * 8; i++) {
        set_free(ta_fake_full_bit_map, i, 1);
    }
    bwrite(FREE_BLOCK_MAP_BLOCK_NUM, ta_fake_full_bit_map);
    CTEST_ASSERT(alloc() == -1, "alloc returns -1 when free block bit map is full");

    // reset image
    image_close();
}


/////  free.c tests  //////////////////////////////////////////////////////////////////////////////


void test_set_and_find_free(void) {

    image_open("test_image", 1);
    mkfs();

    unsigned char tsff_free_block_bit_map[BLOCK_SIZE];
    bread(FREE_BLOCK_MAP_BLOCK_NUM, tsff_free_block_bit_map);
    int first_free_before_set = find_free(tsff_free_block_bit_map);
    CTEST_ASSERT(first_free_before_set == NUM_PRECLAIMED_BLOCKS, "first free block before any setting is block 7"); // 0-6 are allocated by mkfs()

    set_free(tsff_free_block_bit_map, NUM_PRECLAIMED_BLOCKS, 1);
    int first_free_after_set = find_free(tsff_free_block_bit_map);
    CTEST_ASSERT(first_free_after_set == NUM_PRECLAIMED_BLOCKS + 1, "first free block after setting 1 block is 8");

    unsigned char tsff_fake_full_bit_map[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE * 8; i++) {
        set_free(tsff_fake_full_bit_map, i, 1);
    }
    CTEST_ASSERT(find_free(tsff_fake_full_bit_map) == -1, "returns -1 when bit map is full");

    // reset image
    image_close();
}


/////  inode.c tests  /////////////////////////////////////////////////////////////////////////////


/* Project 5 ialloc implementation */

void test_ialloc_old_functionality(void) {

    // image_open("test_image", 1);
    // mkfs();

    // unsigned char ti_free_inode_bit_map[BLOCK_SIZE];
    // bread(FREE_INODE_MAP_BLOCK_NUM, ti_free_inode_bit_map);
    // int first_free_before_ialloc = find_free(ti_free_inode_bit_map);
    // int allocated_inode = ialloc();
    // CTEST_ASSERT(allocated_inode >= 0 && allocated_inode < MAX_NUM_INODES, "inode num returned is contained in the bit array");

    // CTEST_ASSERT(first_free_before_ialloc == allocated_inode, "inode allocated by ialloc is the first free inode");

    // int first_free_after_ialloc = find_free(bread(FREE_INODE_MAP_BLOCK_NUM, ti_free_inode_bit_map));
    // CTEST_ASSERT(first_free_after_ialloc != first_free_before_ialloc, "ialloc takes the first free inode, making it no longer the first free inode");

    // unsigned char ti_fake_full_bit_map[BLOCK_SIZE];
    // for (int i = 0; i < BLOCK_SIZE * 8; i++) {
    //     set_free(ti_fake_full_bit_map, i, 1);
    // }
    // bwrite(FREE_INODE_MAP_BLOCK_NUM, ti_fake_full_bit_map);
    // CTEST_ASSERT(ialloc() == -1, "ialloc returns -1 when free inode bit map is full");

    // // reset image
    // image_close();
}



/* Project 6 ialloc implementation */

void test_ialloc(void) {

    image_open("test_image", 1);
    mkfs();
    reinitialize_incore();

    // the incore inode returned by ialloc has the inode num of the first available inode before ialloc
    unsigned char ti_free_inode_bit_map[BLOCK_SIZE];
    bread(FREE_INODE_MAP_BLOCK_NUM, ti_free_inode_bit_map);
    int ti_first_free_before_ialloc = find_free(ti_free_inode_bit_map);
    struct inode *ti_new_incore_inode = ialloc();
    CTEST_ASSERT(ti_first_free_before_ialloc == (int)ti_new_incore_inode->inode_num, "returns incore inode with inode num of the first available inode before ialloc");

    // the new next free inode is different than it was before ialloc
    bread(FREE_INODE_MAP_BLOCK_NUM, ti_free_inode_bit_map);
    int ti_first_free_after_ialloc = find_free(ti_free_inode_bit_map);
    CTEST_ASSERT(ti_first_free_before_ialloc != ti_first_free_after_ialloc, "the first free inode is different after ialloc");

    // returns null if all incore inodes are in use
    reinitialize_incore();
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {  // sets the first 64 inodes to 'in-use'
        set_free(ti_free_inode_bit_map, i, 1);
    }
    bwrite(FREE_INODE_MAP_BLOCK_NUM, ti_free_inode_bit_map);
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {  // loads the 64 inodes into the 64 incore inodes 
        iget(i);
    }
    struct inode *ti_when_all_incore_full = ialloc();
    CTEST_ASSERT(ti_when_all_incore_full == NULL, "returns null if all incore inodes are being used");
    
    // returns null if no disk inodes are free
    reinitialize_incore();
    for (int i = MAX_SYS_OPEN_FILES; i < BLOCK_SIZE * 8; i++) {  // set the rest of the inodes to 'in-use'
        set_free(ti_free_inode_bit_map, i, 1);
    }
    bwrite(FREE_INODE_MAP_BLOCK_NUM, ti_free_inode_bit_map);
    struct inode *ti_when_all_bit_map_full = ialloc();
    CTEST_ASSERT(ti_when_all_bit_map_full == NULL, "returns null if all inodes in the free inode bit map are already marked as allocated");

    // reset image
    image_close();
}



void test_find_incore_free(void) {

    image_open("test_image", 1);
    mkfs();
    reinitialize_incore();

    struct inode *tfif_incore_inode = find_incore_free();
    CTEST_ASSERT(tfif_incore_inode->ref_count == 0, "incore inode returned has a ref_count of 0");

    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        struct inode *tfif_incore_inode2 = find_incore_free();
        read_inode(find_incore_free(), i);
        tfif_incore_inode2->ref_count += 1;
    }
    CTEST_ASSERT(find_incore_free() == NULL, "returns null when all incore inodes are being used");

    // reset image
    image_close();
}


void test_find_incore(void) {

    image_open("test_image", 1);
    mkfs();
    reinitialize_incore();

    // returns an in-core inode with the inode_num matching the specified inode number
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        struct inode *tfi_incore_inode = find_incore_free();
        read_inode(tfi_incore_inode, i);
        tfi_incore_inode->ref_count += 1;
    }
    int tfi_random_inode_num = rand()%MAX_SYS_OPEN_FILES;
    CTEST_ASSERT((int)find_incore(tfi_random_inode_num)->inode_num == tfi_random_inode_num, "incore inode returned has inode_num matching the specified inode number");

    // returns null if inode is not loaded as an in-core inode
    int inode_num_not_in_incore = MAX_SYS_OPEN_FILES;
    CTEST_ASSERT(find_incore(inode_num_not_in_incore) == NULL, "returns null when inode num does not exist in incore array");

    // reset image
    image_close();
}


void test_read_inode(void) {

    image_open("test_image", 1);
    mkfs();
    reinitialize_incore();

    // inode num is not found before it is read into incore array, and is found afterward
    int tri_random_inode_num = rand()%MAX_NUM_INODES;
    void *result_before_read = find_incore(tri_random_inode_num);
    struct inode *tri_incore_inode = find_incore_free();
    read_inode(tri_incore_inode, tri_random_inode_num);
    tri_incore_inode->ref_count += 1;
    CTEST_ASSERT(result_before_read == NULL && (int)tri_incore_inode->inode_num == tri_random_inode_num, "an inode num is not found before it is read into incore array, but found afterward");

    // reset image
    image_close();
}


void test_write_inode(void) {

    image_open("test_image", 1);
    mkfs();
    reinitialize_incore();

    int twi_random_inode_num = rand()%MAX_NUM_INODES;
    struct inode *twi_incore_inode = find_incore_free();
    read_inode(twi_incore_inode, twi_random_inode_num);
    twi_incore_inode->ref_count += 1;
    int twi_random_size = rand()%UNINCLUSIVE_MAX_INT_W_4_BYTES;
    twi_incore_inode->size = twi_random_size;
    write_inode(twi_incore_inode);
    struct inode *twi_incore_inode2 = find_incore_free();
    read_inode(twi_incore_inode2, twi_random_inode_num);
    CTEST_ASSERT((int)twi_incore_inode2->size == twi_random_size, "write inode saves retrievable data to disk");

    // reset image
    image_close();
}


void test_iget(void) {

    image_open("test_image", 1);
    mkfs();
    reinitialize_incore();

    int tig_random_inode_num = rand()%MAX_NUM_INODES;
    struct inode *result_before_iget = find_incore(tig_random_inode_num);
    struct inode *result_from_first_iget = iget(tig_random_inode_num);
    struct inode *result_after_iget = find_incore(tig_random_inode_num);
    CTEST_ASSERT(result_before_iget == NULL && (int)result_after_iget->inode_num == tig_random_inode_num, "inode num not found in incore array before iget is found after iget");

    struct inode *result_from_second_iget = iget(tig_random_inode_num);
    CTEST_ASSERT(result_from_first_iget == result_from_second_iget, "returns the same incore inode for the inode num as when it first loaded it in");

    reinitialize_incore();
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        iget(i);
    }
    int next_inode_num = MAX_SYS_OPEN_FILES;
    CTEST_ASSERT(iget(next_inode_num) == NULL, "returns null if all incore inodes are being used for other inode nums");

    // reset image
    image_close();
}


void test_iput(void) {

    image_open("test_image", 1);
    mkfs();
    reinitialize_incore();

    int tip_random_inode_num = rand()% MAX_NUM_INODES;
    struct inode *tip_inode = iget(tip_random_inode_num);
    struct inode *result_before_iput = find_incore(tip_random_inode_num);
    iput(tip_inode);
    struct inode *result_after_iput = find_incore(tip_random_inode_num);
    CTEST_ASSERT((int)result_before_iput->inode_num == tip_random_inode_num && result_after_iput == NULL, "inode number found in incore array before iput is not found in incore array after iput");
    
    // was going to also check if trying to iput an already written incore inode just succeeds, but couldn't figure out how to do that with the void function
    // CTEST_ASSERT(iput(tip_inode) != -1, "trying to iput an inode not in incore just doesn't do anything");  // can't check void against int

    // reset image
    image_close();
}


/////  mkfs.c tests  //////////////////////////////////////////////////////////////////////////////


void test_mkfs(void) {

    image_open("test_image", 1);
    mkfs();

    unsigned char tmkfs_random_block[BLOCK_SIZE];
    int random_block_not_preclaimed = NUM_PRECLAIMED_BLOCKS + rand() % (NUM_BLOCKS_IN_FILE_SYS - NUM_PRECLAIMED_BLOCKS);
    bread(random_block_not_preclaimed, tmkfs_random_block);
    int random_byte_in_block = rand() % BLOCK_SIZE;
    CTEST_ASSERT(tmkfs_random_block[random_byte_in_block] == 0, "a random byte from a random block is zeroed (mkfs properly zeroes)");

    unsigned char tmkfs_block_bit_map[BLOCK_SIZE];
    bread(FREE_BLOCK_MAP_BLOCK_NUM, tmkfs_block_bit_map);
    // CTEST_ASSERT(find_free(tmkfs_block_bit_map) == NUM_PRECLAIMED_BLOCKS, "first free block after mkfs() is 7");  // before adding root directory creation to mkfs
    CTEST_ASSERT(find_free(tmkfs_block_bit_map) == NUM_PRECLAIMED_BLOCKS + 1, "first free block after mkfs() is 8");


    // reset image
    image_close();
}


///////////////////////////////////////////////////////////////////////////////////////////////////


/* Assuming that all test functions have been run and all need their leftover image files deleted */

void delete_test_image_files(void) {

    remove("image1");
    remove("image2");
    remove("image_to_close");
    remove("test_image");
}


int main(void) {

    CTEST_VERBOSE(1);

    // // added for project 5
    // test_image_open();
    // test_image_close();
    // test_bread_and_bwrite();
    // test_alloc();
    // test_set_and_find_free();
    test_mkfs();

    // // added for project 6
    // test_find_incore_free();
    // test_find_incore();
    // test_read_inode();
    // test_write_inode();
    // test_iget();
    // test_iput();
    // test_ialloc();

    delete_test_image_files();

    CTEST_RESULTS();

    CTEST_EXIT();

}