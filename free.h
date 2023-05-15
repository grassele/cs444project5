#ifndef FREE_H
#define FREE_H

void set_free(unsigned char *block, int num, int set);   // not sure if we want void to be the return type
int find_free(unsigned char *block);

#endif