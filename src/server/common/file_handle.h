#pragma once

#include <fcntl.h>
#include <memory>
#include <string>

namespace ztofs
{
namespace server
{

class FileHandle
{
public:
    std::unique_ptr<struct file_handle> rawhandle;
    
    FileHandle() {
        rawhandle = std::make_unique<struct file_handle>();
        rawhandle->handle_bytes = 128;
    }
    FileHandle(struct file_handle* handle)
    {
        rawhandle.reset(handle); 
    }
    
    FileHandle& operator=(FileHandle&& other)
    {
        if (this != &other) {
            rawhandle = std::move(other.rawhandle);
        }
        return *this;
    }
};

class FileSystemEnv
{
public:
    int mountfd{-1};
    std::string mountpath;
};

}
}