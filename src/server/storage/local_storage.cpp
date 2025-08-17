#include "local_storage.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>

using namespace ztofs;
using namespace ztofs::server;

butil::Status LocalStorage::Write(FileHandle* fileHandle, const char* buffer, size_t count)
{
    // Validate input parameters
    if (!fileHandle->is_valid) {
        return butil::Status(EINVAL, "Invalid file handle");
    }
    
    if (!buffer && count > 0) {
        return butil::Status(EINVAL, "Buffer is null but count is greater than 0");
    }
    
    if (fileHandle->fd < 0) {
        // TODO: 增加open_by_handle_at 获得fd
        return butil::Status(EBADF, "Invalid file descriptor");
    }
    
    // Perform the write operation
    ssize_t bytesWritten = write(fileHandle->fd, buffer, count);
    if (bytesWritten == -1) {
        return butil::Status(errno, "Failed to write to file: %s", strerror(errno));
    }
    
    // Check if all data was written
    if (static_cast<size_t>(bytesWritten) != count) {
        return butil::Status(EIO, "Incomplete write operation: expected %zu bytes, wrote %zd bytes", 
                            count, bytesWritten);
    }
    
    return butil::Status::OK();
}

butil::Status LocalStorage::Read(FileHandle* fileHandle, char* buffer, size_t count, size_t* bytesRead)
{
    // Validate input parameters
    if (!fileHandle->is_valid) {
        return butil::Status(EINVAL, "Invalid file handle");
    }
    
    if (!buffer && count > 0) {
        return butil::Status(EINVAL, "Buffer is null but count is greater than 0");
    }
    
    if (fileHandle->fd < 0) {
        // TODO: 增加open_by_handle_at 获得fd
        return butil::Status(EBADF, "Invalid file descriptor");
    }
    
    if (!bytesRead) {
        return butil::Status(EINVAL, "Bytes read output parameter is null");
    }
    
    // Perform the read operation
    ssize_t bytesReadResult = read(fileHandle->fd, buffer, count);
    if (bytesReadResult == -1) {
        return butil::Status(errno, "Failed to read from file: %s", strerror(errno));
    }
    
    // Set the number of bytes actually read
    *bytesRead = static_cast<size_t>(bytesReadResult);
    
    return butil::Status::OK();
}