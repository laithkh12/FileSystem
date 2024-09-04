//
// Created by leth1 on 7/4/2024.
//

#ifndef FSYSTEM_BLKDEV_H
#define FSYSTEM_BLKDEV_H
#include <string>

class BlockDeviceSimulator {
public:
    BlockDeviceSimulator(std::string fname);
    ~BlockDeviceSimulator();

    void read(int addr, int size, char *ans);
    void write(int addr, int size, const char *data);

    static const int DEVICE_SIZE = 1024 * 1024;

private:
    int fd;
    unsigned char *filemap;
};
#endif //FSYSTEM_BLKDEV_H