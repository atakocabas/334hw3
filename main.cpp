#include "main.h"


std::ifstream ext2_image;
ext2_super_block super_block;
ext2_block_group_descriptor group_descriptor;
std::vector<std::string> path_vector;
unsigned int block_size;
unsigned int inode_size;
unsigned char* block_bitmap;
unsigned char* inode_bitmap;
ext2_inode* curr_inode = NULL;
unsigned int curr_inode_id;
#define GET_BLOCK_OFFSET(block) ((block)*block_size)

void read_image(std::string image_path){
    ext2_image.open(image_path, std::ios::binary);

    if(!ext2_image.is_open()) {
        std::cout << "COULD NOT OPEN THE IMAGE FILE!" << std::endl;
        exit(14);
    }
}

void read_superblock() {
    ext2_image.seekg(EXT2_SUPER_BLOCK_POSITION, std::ios::beg);
    ext2_image.read((char*)&super_block, sizeof(super_block));
    if(EXT2_SUPER_MAGIC != super_block.magic){
        std::cout << "MAGIC DOES NOT MATCH!" << std::endl;
        exit(1);
    }
    block_size = EXT2_UNLOG(super_block.log_block_size);
}

void read_group_descriptor(){
    ext2_image.seekg(block_size, std::ios::beg);
    ext2_image.read((char*)&group_descriptor, sizeof(ext2_block_group_descriptor));
}

void path_tokenizer(std::string path) {
    // TODO: tokenize the path, then put into path vector
    std::string temp = "";
    for(size_t i=0; i < path.size()+1; i++) {
        if(path[i] == '/' || path[i] == '\0'){
            if(temp != ""){
                path_vector.push_back(temp);
            }
            temp = "";
            continue;
        } else {
            temp += path[i];
        }
    }
}

std::vector<ext2_dir_entry*> get_path_dirs(ext2_inode* inode) {
    std::vector<ext2_dir_entry*> dirs;
    unsigned int size = 0;

    while(size < inode->size) {
        unsigned int offset = GET_BLOCK_OFFSET(inode->direct_blocks[0]) + size;
        ext2_dir_entry* tmp = new ext2_dir_entry;
        ext2_image.seekg(offset, std::ios::beg);
        ext2_image.read((char*)tmp, sizeof(ext2_dir_entry));

        ext2_image.seekg(offset + sizeof(ext2_dir_entry), std::ios::beg);
        ext2_image.read((char*) tmp->name, tmp->name_length + 1);
        if(tmp->file_type == EXT2_D_DTYPE){
            dirs.push_back(tmp);
        }
        size += tmp->length;
    }

    return dirs;
}

ext2_dir_entry* find_in_dirs(std::string name, std::vector<ext2_dir_entry*> dirs) {
    for(auto dir: dirs) {
        if(strcmp(dir->name, name.c_str()) == 0) return dir;
    }
    return NULL;
}

bool check_path_exists(){
    // TODO: check if path vector exist
    ext2_inode* root_inode = get_inode(EXT2_ROOT_INODE);
    unsigned int root_inode_id;
    bool res = false;
    for(size_t i=0; i < path_vector.size(); ++i) {
        std::vector<ext2_dir_entry*> dirs = get_path_dirs(root_inode);
        root_inode_id = dirs[0]->inode;
        if(path_vector[path_vector.size()-1] == path_vector[i]){
            if(find_in_dirs(path_vector[i], dirs) != NULL) {
                res = false;
                break;
            }
            else {
                res = true;
                break;
            }
        } else {
            ext2_dir_entry* curr_dir = find_in_dirs(path_vector[i], dirs);
            if(curr_dir != NULL) {
                root_inode = get_inode(curr_dir->inode);
            } else {
                res = false;
                break;
            }
        }
    }
    curr_inode = root_inode;
    curr_inode_id = root_inode_id;
    return res;
}

void print_dir_entries(unsigned int index) {
    ext2_inode* inode = get_inode(index);
    std::vector<ext2_dir_entry*> dirs = get_path_dirs(inode);
    for(auto dir: dirs) {
        print_dir_entry(dir, dir->name);
    }
}

ext2_inode* get_parent_inode(ext2_inode* inode) {
    // TODO: get the parent inode
    unsigned int offset = GET_BLOCK_OFFSET(inode->direct_blocks[0]) + 12;
    ext2_dir_entry* parent_dir_entry = new ext2_dir_entry;
    ext2_image.seekg(offset, std::ios::beg);
    ext2_image.read((char*)parent_dir_entry, sizeof(ext2_dir_entry));
    print_inode(get_inode(parent_dir_entry->inode), parent_dir_entry->inode);
    return get_inode(parent_dir_entry->inode);
}

ext2_inode* get_inode(int index) {
    // TODO: get the inode of given index
    ext2_inode* res = new ext2_inode;
    unsigned int offset = GET_BLOCK_OFFSET(group_descriptor.inode_table) + (index - 1) * super_block.inode_size;
    ext2_image.seekg(offset, std::ios::beg);
    ext2_image.read((char*) res, sizeof(ext2_inode));
    return res;
}

void read_bitmaps() {
    block_bitmap = new unsigned char[block_size];
    inode_bitmap = new unsigned char[block_size];

    ext2_image.seekg(GET_BLOCK_OFFSET(group_descriptor.block_bitmap), std::ios::beg);
    ext2_image.read((char*)block_bitmap, block_size);

    ext2_image.seekg(GET_BLOCK_OFFSET(group_descriptor.inode_bitmap), std::ios::beg);
    ext2_image.read((char*)inode_bitmap, block_size);
}

unsigned int find_first_empty_index(){
    for(unsigned int i=0; i < block_size; ++i) {
        unsigned char byte = inode_bitmap[i];
        for(int j=0; j < 8; j++){
            bool inodeUsed = byte & (1 << j);
            if(inodeUsed) {
                std::cout << i * 8 + j << " bit is used!" << std::endl;
            }
        }
    }
    return 0;
}

ext2_inode* allocate_inode() {
    unsigned int first_empty_index = find_first_empty_index();
    return NULL;
}
int main(int argc, char const *argv[])
{
    if(argc < 2) return -1;
    std::string image_path = argv[1];
    std::string command = argv[2];

    read_image(image_path);
    read_superblock();
    read_group_descriptor();
    read_bitmaps();
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
        if(argc < 4){
            std::cout << "DO NOT FORGET TO ADD THE PATH!" << std::endl;
            exit(1);
        }
        path_tokenizer(argv[3]);
        if(check_path_exists() == false){
            std::cout << "PATH ALREADY EXISTS!" << std::endl;
            exit(1);
        }

        ext2_inode* parent_inode = curr_inode;
        unsigned int parent_inode_id = curr_inode_id;
        print_inode(get_inode(41), 41);
        print_dir_entries(41);
        ext2_inode* new_inode = allocate_inode();
    }

    ext2_image.close();
    return 0;
}