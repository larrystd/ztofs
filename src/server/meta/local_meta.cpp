#include "local_meta.h"
#include <fcntl.h>

using namespace ztofs;
using namespace ztofs::server;


butil::Status LocalMeta::Create(const std::string& path, FileHandle* fileHandle)
{
    if (!fileHandle || !(fileHandle->is_valid)) {
        return butil::Status(EINVAL, "Invalid file handle");
    }
    
    int fd = open(path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        return butil::Status(errno, "Failed to create file: %s", strerror(errno));
    }
    
    *fileHandle = FileHandle(path, fd);
    return butil::Status::OK();
}

butil::Status LocalMeta::Remove(const FileHandle& fileHandle)
{
    if (!fileHandle.is_valid) {
        return butil::Status(EINVAL, "Invalid file handle");
    }
    
    if (fileHandle.fd >= 0) {
        close(fileHandle.fd);
    }
    
    if (unlink(fileHandle.path.c_str()) == -1) {
        return butil::Status(errno, "Failed to remove file: %s", strerror(errno));
    }
    
    return butil::Status::OK();
}
