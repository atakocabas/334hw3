#include <iostream>
#include <string.h>
#include <fstream>
#include "ext2fs.h"
#include "ext2fs_print.h"
#include <vector>
#include <math.h>
#include <sys/stat.h>
#include <algorithm>

ext2_inode* get_inode(int);

ext2_inode* allocate_inode(unsigned int);

unsigned int find_first_empty_inode_index();