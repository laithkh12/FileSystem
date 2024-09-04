#include "myfs.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <algorithm>

const char *MyFs::MYFS_MAGIC = "MYFS";
const int MyFs::BLOCK_SIZE = 4096;
const int MyFs::FILE_LIST_BLOCK = 1;

MyFs::MyFs(BlockDeviceSimulator *blkdevsim_) : blkdevsim(blkdevsim_) {
    myfs_header header;
    blkdevsim->read(0, sizeof(header), (char *)&header);

    if (strncmp(header.magic, MYFS_MAGIC, sizeof(header.magic)) != 0 || (header.version != CURR_VERSION)) {
        std::cout << "Did not find myfs instance on blkdev" << std::endl;
        std::cout << "Creating..." << std::endl;
        format();
        std::cout << "Finished!" << std::endl;
    } else {
        load_file_list();
        for (const auto& f : file_list) {
            std::cout << f << std::endl;
        }
    }
}

void MyFs::format() {
    myfs_header header;
    strncpy(header.magic, MYFS_MAGIC, sizeof(header.magic));
    header.version = CURR_VERSION;
    blkdevsim->write(0, sizeof(header), (const char*)&header);

    file_list.clear();
    update_file_list();
}

void MyFs::create_file(std::string path_str, bool directory) {
    // Check if the file already exists
    auto it = find_file(path_str);
    if (it != file_list.end()) {
        std::cout << "Error: File '" << path_str << "' already exists." << std::endl;
        return;
    }

    // Add the file to file_list
    file_list.push_back(path_str);

    // Update file list on the block device
    update_file_list();

    // Calculate index of the new file in file_list
    int index = file_list.size() - 1;

    // Write an empty block to the block device only if it's a new file
    if (index >= 0) {
        char empty_block[BLOCK_SIZE] = {0};
        blkdevsim->write(index * BLOCK_SIZE, BLOCK_SIZE, empty_block);
    } else {
        std::cout << "Error: Invalid index " << index << " for file '" << path_str << "'." << std::endl;
    }
}

std::string MyFs::get_content(std::string path_str) {
    auto it = find_file(path_str);
    if (it != file_list.end()) {
        int index = std::distance(file_list.begin(), it);
        char content[BLOCK_SIZE] = {0}; // Initialize content buffer to zero
        blkdevsim->read(index * BLOCK_SIZE, BLOCK_SIZE, content);
        return std::string(content);
    } else {
        std::cout << "Error: File '" << path_str << "' not found." << std::endl;
        return "";
    }
}

void MyFs::set_content(std::string path_str, std::string content) {
    auto it = find_file(path_str);
    if (it != file_list.end()) {
        int index = std::distance(file_list.begin(), it);
        char content_buffer[BLOCK_SIZE] = {0};
        strncpy(content_buffer, content.c_str(), BLOCK_SIZE);
        blkdevsim->write(index * BLOCK_SIZE, BLOCK_SIZE, content_buffer);
    } else {
        std::cout << "Error: File '" << path_str << "' not found. Cannot edit." << std::endl;
    }
}

void MyFs::list_dir(std::string path_str) {
    for (const auto& file : file_list) {
        if (path_str == "/" || file.find(path_str + "/") == 0 || file == path_str) {
            size_t file_size = calculate_file_size(file);
            std::cout << file << '\t' << file_size << std::endl;
        }
    }
}

void MyFs::remove_file(std::string path_str) {
    auto it = find_file(path_str);
    if (it != file_list.end()) {
        int index = std::distance(file_list.begin(), it);
        file_list.erase(it); // Remove from file_list
        update_file_list();  // Update block device
        char empty_block[BLOCK_SIZE] = {0};
        blkdevsim->write(index * BLOCK_SIZE, BLOCK_SIZE, empty_block);
        std::cout << "Removed file: " << path_str << std::endl;
    } else {
        std::cout << "Error: File '" << path_str << "' not found." << std::endl;
    }
}

void MyFs::update_file_list() {
    std::ostringstream oss;
    for (const auto& file : file_list) {
        oss << file << "\n";
    }
    std::string file_list_str = oss.str();
    blkdevsim->write(FILE_LIST_BLOCK * BLOCK_SIZE, file_list_str.size(), file_list_str.c_str());
}

size_t MyFs::calculate_file_size(const std::string& path_str) {
    auto it = find_file(path_str);
    if (it != file_list.end()) {
        int index = std::distance(file_list.begin(), it);
        char content[BLOCK_SIZE] = {0};
        blkdevsim->read(index * BLOCK_SIZE, BLOCK_SIZE, content);
        return strlen(content);
    }
    return 0;
}

void MyFs::mkdir(std::string dir_name) {
    if (find_file(dir_name) != file_list.end()) {
        std::cout << dir_name << " already exists." << std::endl;
        return;
    }

    file_list.push_back(dir_name);
    update_file_list();
    char empty_block[BLOCK_SIZE] = {0};
    blkdevsim->write(file_list.size() * BLOCK_SIZE, BLOCK_SIZE, empty_block);
}

void MyFs::mv(std::string old_path, std::string new_path) {
    auto it = find_file(old_path);
    if (it == file_list.end()) {
        std::cout << "Error: File or directory '" << old_path << "' not found." << std::endl;
        return;
    }
    if (find_file(new_path) != file_list.end()) {
        std::cout << "Error: '" << new_path << "' already exists." << std::endl;
        return;
    }
    *it = new_path;
    update_file_list();
    std::cout << "Moved '" << old_path << "' to '" << new_path << "'." << std::endl;
}


void MyFs::rmdir(std::string dir_name) {
    auto it = find_file(dir_name);
    if (it == file_list.end()) {
        std::cout << dir_name << " not found." << std::endl;
        return;
    }
    file_list.erase(it); // Remove from file_list
    update_file_list();  // Update block device
    for (auto it = file_list.begin(); it != file_list.end(); ) {
        if (it->find(dir_name + "/") == 0 || *it == dir_name) {
            it = file_list.erase(it);
        } else {
            ++it;
        }
    }
    char empty_block[BLOCK_SIZE] = {0};
    blkdevsim->write(file_list.size() * BLOCK_SIZE, BLOCK_SIZE, empty_block);
    std::cout << dir_name << " removed and all its contents." << std::endl;
}



void MyFs::load_file_list() {
    char file_list_block[BLOCK_SIZE] = {0};
    blkdevsim->read(FILE_LIST_BLOCK * BLOCK_SIZE, BLOCK_SIZE, file_list_block);
    std::istringstream iss(file_list_block);
    std::string file;
    while (std::getline(iss, file)) {
        if (!file.empty()) {
            file_list.push_back(file);
        }
    }
}
std::vector<std::string>::iterator MyFs::find_file(const std::string& path_str) {
    return std::find(file_list.begin(), file_list.end(), path_str);
}