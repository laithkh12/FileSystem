//
// Created by leth1 on 7/4/2024.
//

#ifndef FSYSTEM_MYFS_H
#define FSYSTEM_MYFS_H



#include <memory>
#include <vector>
#include <stdint.h>
#include "blkdev.h"

class MyFs {
public:
    MyFs(BlockDeviceSimulator *blkdevsim_);

    /**
     * format method
     * This function discards the current content in the blockdevice and
     * create a fresh new MYFS instance in the blockdevice.
     */
    void format();

    /**
     * create_file method
     * Creates a new file in the required path.
     * @param path_str the file path (e.g. "/newfile")
     * @param directory boolean indicating whether this is a file or directory
     */
    void create_file(std::string path_str, bool directory);

    /**
     * get_content method
     * Returns the whole content of the file indicated by path_str param.
     * Note: this method assumes path_str refers to a file and not a
     * directory.
     * @param path_str the file path (e.g. "/somefile")
     * @return the content of the file
     */
    std::string get_content(std::string path_str);

    /**
     * set_content method
     * Sets the whole content of the file indicated by path_str param.
     * Note: this method assumes path_str refers to a file and not a
     * directory.
     * @param path_str the file path (e.g. "/somefile")
     * @param content the file content string
     */
    void set_content(std::string path_str, std::string content);

    /**
      * list_dir method
      * Returns a list of a files in a directory.
      * Note: this method assumes path_str refers to a directory and not a
      * file.
      * @param path_str the file path (e.g. "/somedir")
      * @return a vector (you need to change the return type in the function declaration)
      */
    void list_dir(std::string path_str);
    void remove_file(std::string path_str);

    void mkdir(std::string dir_name);

    void rmdir(std::string dir_name);

    void mv(std::string old_path, std::string new_path);

private:

    /**
     * This struct represents the first bytes of a myfs filesystem.
     * It holds some magic characters and a number indicating the version.
     * Upon class construction, the magic and the header are tested - if
     * they both exist than the file is assumed to contain a valid myfs
     * instance. Otherwise, the blockdevice is formated and a new instance is
     * created.
     */
    struct myfs_header {
        char magic[4];
        uint8_t version;
    };


    BlockDeviceSimulator *blkdevsim;

    static const uint8_t CURR_VERSION = 0x03;
    static const char *MYFS_MAGIC;
    std::vector<std::string> file_list;
    void update_file_list();

    size_t calculate_file_size(const std::string &path_str);

};



#endif //FSYSTEM_MYFS_H