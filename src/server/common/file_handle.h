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
    
    FileHandle() : fd(-1), is_valid(true) {}
    FileHandle(const std::string& path, int fd) 
        : path(path), fd(fd), is_valid(true) {}

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