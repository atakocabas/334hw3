#include "main.h"


std::fstream ext2_image;
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
    ext2_image.open(image_path, std::fstream::in | std::fstream::out);

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

unsigned int find_first_empty_inode_index(){
    for(unsigned int i=0; i < block_size; ++i) {
        unsigned char byte = inode_bitmap[i];
        for(int j=0; j < 8; j++){
            bool inodeUsed = byte & (1 << j);
            if(!inodeUsed) {
                return i*8+j+1;
            }
        }
    }
    return 0;
}

ext2_inode* allocate_inode(unsigned int empty_inode_index) {
    ext2_inode* empty_inode = get_inode(empty_inode_index);
    return empty_inode;
}

int allocate_block() {
    for(uint32_t i=0; i < block_size; ++i) {
        unsigned char byte = block_bitmap[i];
        for(uint32_t j=0; j < 8; ++j) {
            bool blockUsed = byte & (1 << j);
            if(!blockUsed){
                return i*8+j;
            }
        }
    }
    return -1;
}

ext2_dir_entry* create_dir_entry(const std::string& name, uint32_t inode_index){
    size_t name_length = name.length();
    size_t entry_size = sizeof(ext2_dir_entry) + name_length + 4 - name_length % 4;
    ext2_dir_entry* new_dir_entry = (ext2_dir_entry*)malloc(entry_size);
    
    new_dir_entry->inode = inode_index;
    new_dir_entry->name_length = static_cast<uint8_t>(name_length);
    new_dir_entry->length = static_cast<uint16_t>(entry_size);
    new_dir_entry->file_type = EXT2_D_DTYPE;
    memcpy(new_dir_entry->name, name.c_str(), name_length+1);
    return new_dir_entry;
}

void write_inode_to_img(ext2_inode* inode, unsigned int id) {
    unsigned int offset = GET_BLOCK_OFFSET(group_descriptor.inode_table) + (id - 1) * super_block.inode_size;
    ext2_image.seekp(offset, std::ios::beg);
    ext2_image.write((char*) inode, sizeof(ext2_inode));
}

void write_dir_to_parent_dirs(ext2_dir_entry* dir, ext2_inode* parent) {
    std::vector<ext2_dir_entry*> parent_dirs = get_path_dirs(parent);
    unsigned int total_parent_dirs_size = 0;
    for(size_t i=0; i < parent_dirs.size() - 1; ++i){
        total_parent_dirs_size += parent_dirs[i]->length;
    }
    unsigned int last_new_size = parent_dirs[parent_dirs.size()-1]->name_length + (4 - parent_dirs[parent_dirs.size()-1]->name_length % 4) + 8;
    total_parent_dirs_size += last_new_size;
    parent_dirs[parent_dirs.size()-1]->length = last_new_size;
    dir->length = block_size - total_parent_dirs_size;
    parent_dirs.push_back(dir);

    unsigned int size = 0;
    unsigned int index = 0;
    while(size < parent->size) {
        unsigned int offset = GET_BLOCK_OFFSET(parent->direct_blocks[0]) + size;
        ext2_image.seekp(offset, std::ios::beg);
        ext2_image.write((char*)parent_dirs[index], parent_dirs[index]->length);
        size += parent_dirs[index]->length;
        index++;
    }

}

void write_self_dirs(std::vector<ext2_dir_entry*> dirs, ext2_inode* self, unsigned int id){
    unsigned int offset = GET_BLOCK_OFFSET(self->direct_blocks[0]);
    ext2_image.seekp(offset, std::ios::beg);
    ext2_image.write((char*)dirs[0], dirs[0]->length);

    offset += dirs[0]->length;

    ext2_image.seekp(offset, std::ios::beg);
    ext2_image.write((char*)dirs[1], dirs[1]->length);
}

void update_time_parent_to_root(unsigned int parent_id){
    time_t t;
    ext2_inode* tmp = get_inode(parent_id);
    tmp->modification_time = time(&t);
    tmp->access_time = t;
    unsigned int offset = GET_BLOCK_OFFSET(group_descriptor.inode_table) + (parent_id-1) * super_block.inode_size;
    ext2_image.seekp(offset, std::ios::beg);
    ext2_image.write((char*)tmp, sizeof(ext2_inode));

    while(true) {
        std::vector<ext2_dir_entry*> dirs = get_path_dirs(tmp);
        if(dirs[1]->inode == parent_id) break;
        parent_id = dirs[1]->inode;
        tmp = get_inode(parent_id);
        tmp->modification_time = t;
        tmp->access_time = t;
        offset = GET_BLOCK_OFFSET(group_descriptor.inode_bitmap) + (parent_id - 1) * super_block.inode_size;
        ext2_image.seekp(offset, std::ios::beg);
        ext2_image.write((char*)tmp, sizeof(ext2_inode));
    }
}

void update_bitmaps(){
    bool flag = true;
    for(unsigned int i=0; i < block_size || flag; ++i) {
        unsigned char byte = inode_bitmap[i];
        for(int j=0; j < 8 || flag; j++){
            bool inodeUsed = byte & (1 << j);
            if(!inodeUsed) {
                inode_bitmap[i] = byte | (1 << j);
                flag = false;
            }
        }
    }

    ext2_image.seekp(GET_BLOCK_OFFSET(group_descriptor.inode_bitmap), std::ios::beg);
    ext2_image.write((char*)inode_bitmap, block_size);

    flag = true;
    for(unsigned int i=0; i < block_size || flag; ++i){
        unsigned char byte = block_bitmap[i];
        for(unsigned int j=0; j < 8 || flag; ++j){
            bool blockUsed = byte & (1 << j);
            if(!blockUsed){
                block_bitmap[i] = byte | (1 << j);
                flag = false;
            }
        }
    }
}

ext2_inode* find_inode_in_dirs(std::vector<ext2_dir_entry*> dirs, std::string name){
    for(auto dir: dirs){
        if(strcmp(dir->name, name.c_str()) == 0) {
            return get_inode(dir->inode);
        }
    }
    return NULL;
}

ext2_inode* check_rmdir_path_exists(){
    ext2_inode* tmp_inode = get_inode(EXT2_ROOT_INODE);
    for(size_t i=0; i < path_vector.size(); ++i){
        std::vector<ext2_dir_entry*> dirs = get_path_dirs(tmp_inode);
        if(path_vector[path_vector.size()-1] == path_vector[i]){
            ext2_inode* tmp = find_inode_in_dirs(dirs, path_vector[i]);
            if(tmp != NULL) {
                std::vector<ext2_dir_entry*> tmp_dirs = get_path_dirs(tmp);
                if(tmp_dirs.size() > 2) {
                    std::cout << "DIRECTORY IS NOT EMPTY!" << std::endl;
                    exit(1);
                } else {
                    return tmp;
                }
            } else {
                std::cout << "COULD NOT FIND THE PATH!" << std::endl;
                exit(1);
            }
        } else {
            ext2_inode* tmp = find_inode_in_dirs(dirs, path_vector[i]);
            if(tmp != NULL){
                tmp_inode = tmp;
            } else {
                std::cout << "COULD NOT FIND PATH!" << std::endl;
                exit(1);
            }
        }
   }

   return NULL;
}

unsigned int get_inode_id(ext2_inode* inode) {
    std::vector<ext2_dir_entry*> dirs = get_path_dirs(inode);
    return dirs[0]->inode;
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
        print_dir_entries(2);
        delete tmp_inode;
    } else if (command == "super") {
        print_super_block(&super_block);
    } else if(command == "group"){
        print_group_descriptor(&group_descriptor);
    } else if (command == "mkdir") {
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
        unsigned int new_inode_index = find_first_empty_inode_index();
        ext2_inode* new_inode = allocate_inode(new_inode_index);
        int empty_block_index = allocate_block();
        if(empty_block_index == -1){
            std::cout << "COULD NOT ALLOCATED A BLOCK!" << std::endl;
            exit(1);
        }

        new_inode->direct_blocks[0] = empty_block_index;
        new_inode->mode = EXT2_I_DTYPE + EXT2_I_DPERM;
        new_inode->gid = EXT2_I_GID;
        new_inode->size = block_size;
        time_t t;
        new_inode->access_time = time(&t);
        new_inode->creation_time = t;
        new_inode->modification_time = t;
        new_inode->uid = EXT2_I_UID;
        new_inode->link_count = 1;
        new_inode->block_count_512 = 8;
        ext2_dir_entry* new_dir_entry = create_dir_entry(path_vector[path_vector.size()-1], new_inode_index);
        ext2_dir_entry* dot_entry = create_dir_entry(".", new_inode_index);
        ext2_dir_entry* dotdot_entry = create_dir_entry("..", parent_inode_id);
        dotdot_entry->length = block_size - 12;
        std::vector<ext2_dir_entry*> dirs;
        dirs.push_back(dot_entry);
        dirs.push_back(dotdot_entry);

        write_inode_to_img(new_inode, new_inode_index);
        write_dir_to_parent_dirs(new_dir_entry, parent_inode);
        write_self_dirs(dirs, new_inode, new_inode_index);
        update_bitmaps();
    } else if(command == "rmdir") {
        if(argc < 4){
            std::cout << "DO NOT FORGET TO ADD THE PATH!" << std::endl;
            exit(1);
        }
        path_tokenizer(argv[3]);
        ext2_inode* rm_inode = check_rmdir_path_exists();
        unsigned int rm_inode_id = get_inode_id(rm_inode);
        print_inode(rm_inode, rm_inode_id);
    }
    ext2_image.close();
    return 0;
}