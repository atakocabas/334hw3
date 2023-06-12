#include <iostream>
#include <string.h>
#include <fstream>
#include "ext2fs.h"
#include "ext2fs_print.h"

std::ifstream ext2_image;
ext2_super_block super_block;

void read_image(std::string image_path){
    ext2_image.open(image_path, std::ios::binary);

    if(!ext2_image.is_open()) {
        std::cout << "COULD NOT OPEN THE IMAGE FILE!" << std::endl;
    }
}

void read_superblock() {
    ext2_image.seekg(1024, std::ios::beg);
    ext2_image.read((char*)&super_block, sizeof(super_block));
}

void read_inode(ext2_inode* inode, int index) {
    // TODO: read the inode from the corresponding index
    
}

int main(int argc, char const *argv[])
{
    if(argc < 2) return -1;
    std::string image_path = argv[1];
    std::string command = argv[2];

    read_image(image_path);
    if(command == "inode"){
        
    } else if (command == "superblock") {
        read_superblock();
        print_super_block(&super_block);
    }
    return 0;
}
