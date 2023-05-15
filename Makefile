# Build the static library and main C testing file into an executable

simfs_test: simfs_test.o simfs.a
	gcc -Wall -Wextra -o $@ $^

.PHONY: test

test: simfs_test
    ./simfs_test



# Build the object files into a static library called simfs.a

simfs.a: block.o free.o image.o mkfs.o    # does not include simfs_test.o
	ar rcs $@ $^



# Build C files to object files

block.o: block.c
	gcc -Wall -Wextra -c $@ $^

free.o: free.c
	gcc -Wall -Wextra -c $@ $^

image.o: image.c
	gcc -Wall -Wextra -c $@ $^

mkfs.o: mkfs.c
	gcc -Wall -Wextra -c $@ $^

simfs_test.o: simfs_test.c
	gcc -Wall -Wextra -c $@ $^