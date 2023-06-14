#include <iostream>
#include <string.h>
#include <fstream>
#include "ext2fs.h"
#include "ext2fs_print.h"
#include <vector>
#include <math.h>

const uint32_t ROOT_INODE = 2;
const uint16_t EXT2_S_IFDIR = 0x4000;

std::ifstream ext2_image;
ext2_super_block super_block;
ext2_block_group_descriptor group_descriptor;
std::vector<std::string> path_vector;
unsigned int block_size;
unsigned char* block_bitmap;
unsigned char* inode_bitmap;

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

void read_group_descriptor(){
    ext2_image.seekg(block_size, std::ios::beg);
    ext2_image.read((char*)&group_descriptor, sizeof(ext2_block_group_descriptor));
}

void path_tokenizer(std::string path) {
    // TODO: tokenize the path, then put into path vector
    std::string temp = "";
    for(auto e: path) {
        if(e == '/'){
            if(temp != ""){
                path_vector.push_back(temp);
            }
            temp = "";
            continue;
        } else {
            temp += e;
        }
    }
}

void calculate_block_size() {
    block_size = 1024 << super_block.log_block_size;
}

bool check_path_exist(){
    // TODO: check if path vector exist
    return false;
}

ext2_inode* get_parent_inode() {
    // TODO: get the parent inode
    return NULL;
}

ext2_inode* get_inode(int index) {
    // TODO: get the inode of given index
    ext2_inode* res = new ext2_inode;
    unsigned int offset = (1024 + (group_descriptor.inode_table - 1) * block_size) + (index - 1) * sizeof(ext2_inode);
    ext2_image.seekg(offset, std::ios::beg);
    ext2_image.read((char*) res, sizeof(ext2_inode));
    return res;
}

void read_bitmaps() {
    block_bitmap = new unsigned char[block_size];
    inode_bitmap = new unsigned char[block_size];

    ext2_image.seekg(1024 + (group_descriptor.block_bitmap - 1) * block_size, std::ios::beg);
    ext2_image.read((char*)block_bitmap, block_size);

    ext2_image.seekg(1024 + (group_descriptor.inode_bitmap - 1) * block_size, std::ios::beg);
    ext2_image.read((char*)inode_bitmap, block_size);
}

int main(int argc, char const *argv[])
{
    if(argc < 2) return -1;
    std::string image_path = argv[1];
    std::string command = argv[2];

    read_image(image_path);
    read_superblock();
    calculate_block_size();
    read_group_descriptor();
    if(command == "inode"){
        // TODO: print the inode
        ext2_inode* tmp_inode = get_inode(atoi(argv[3]));
        print_inode(tmp_inode, atoi(argv[3]));
    } else if (command == "super") {
        print_super_block(&super_block);
    } else if(command == "group"){
        print_group_descriptor(&group_descriptor);
    }else if (command == "mkdir") {
        // TODO: mkdir
        path_tokenizer(argv[3]);
        if(check_path_exist()){
            std::cout << "PATH ALREADY EXISTS!" << std::endl;
            return -1;
        }

        ext2_inode* parent_inode = get_parent_inode();

    }
    return 0;
}