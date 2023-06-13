#include <iostream>
#include <string.h>
#include <fstream>
#include "ext2fs.h"
#include "ext2fs_print.h"
#include <vector>

std::ifstream ext2_image;
ext2_super_block super_block;
std::vector<std::string> path_vector;

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
    // TODO: read the inode from the corresponding index into the inode

}

void path_tokenizer(std::string path) {
    // TODO: tokenize the path, then put into path vector
    std::string token = path.substr(0, path.find("/"));
}

bool check_path_exist(){
    // TODO: check if path vector exist
    return false;
}

ext2_inode* get_parent_inode() {
    // TODO: get the parent inode
    return NULL;
}

int main(int argc, char const *argv[])
{
    if(argc < 2) return -1;
    std::string image_path = argv[1];
    std::string command = argv[2];

    read_image(image_path);
    if(command == "inode"){
        // TODO: print the inode
        ext2_inode* tmp_inode = new ext2_inode;
        read_inode(tmp_inode, (int) argv[3]);
    } else if (command == "superblock") {
        read_superblock();
        print_super_block(&super_block);
    } else if (command == "mkdir") {
        // TODO: mkdir
        path_tokenizer(argv[3]);
        if(!check_path_exist()){
            std::cout << "PATH ALREADY EXISTS!" << std::endl;
            return -1;
        }

        ext2_inode* parent_inode = get_parent_inode();

    }
    return 0;
}
