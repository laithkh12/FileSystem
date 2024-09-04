cmake_minimum_required(VERSION 3.28)
project(fsystem)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_executable(fileSystem
        blkdev.h
        blkdev.cpp
        myfs.h
        myfs.cpp
        myfs_main.cpp
        vfs.cpp
)
