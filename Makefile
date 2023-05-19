# Build the static library and main C testing file into an executable

simfs_test: simfs_test.c simfs.a
	gcc -Wall -Wextra -o $@ $^

.PHONY: test

test: simfs_test
	./simfs_test



# Build the object files into a static library called simfs.a

simfs.a: block.o free.o image.o mkfs.o
	ar rcs $@ $^



# Build C files to object files

block.o: block.c image.o free.o
	gcc -Wall -Wextra -c -o $@ $<

free.o: free.c block.o
	gcc -Wall -Wextra -c -o $@ $<

image.o: image.c
	gcc -Wall -Wextra -c -o $@ $<

mkfs.o: mkfs.c block.o
	gcc -Wall -Wextra -c -o $@ $<