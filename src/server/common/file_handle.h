#pragma once

#include <fcntl.h>
#include <string>

namespace ztofs
{
namespace server
{

struct FileHandle 
{
    std::string path;
    int fd{-1};
    bool is_valid{true};
    struct file_handle* handle;
    
    FileHandle() : fd(-1), is_valid(true) {}
    FileHandle(const std::string& path, int fd) 
        : path(path), fd(fd), is_valid(true) {}
    
    FileHandle(const std::string& path, int fd, struct file_handle* handle) 
        : path(path), fd(fd), is_valid(true), handle(handle) {}

    FileHandle& operator=(const FileHandle& other)
    {
        if (this != &other) {
            path = other.path;
            fd = other.fd;
            is_valid = other.is_valid;
        }
        return *this;
    }
};

}
}