#include "local_storage.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <butil/logging.h>
#include <gflags/gflags.h>

using namespace ztofs;
using namespace ztofs::server;

DEFINE_bool(enable_sync, false, "enable sync when write");

butil::Status LocalStorage::Write(const FileHandle& fileHandle, const char* buffer, size_t count, size_t offset, size_t* bytesWritten)
{   
    int fd = open_by_handle_at(mFsEnv->mountfd, fileHandle.RawHandle(), O_RDWR);
    if (fd < 0) {
        LOG(ERROR) << "Failed to open  handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open handle: %s", strerror(errno));
    }
    // printf("write buffer: %s\n", buffer);
    *bytesWritten = pwrite(fd, buffer, count, offset);
    // LOG(INFO) << "bytes written: " << *bytesWritten;
    if (FLAGS_enable_sync && fsync(fd) == -1)
    {
        close(fd);
        LOG(ERROR) << "fsync failed: " << strerror(errno);
        return butil::Status(errno, "%s", strerror(errno));
    }

    close(fd);
    return butil::Status::OK();
}

butil::Status LocalStorage::Read(const FileHandle& fileHandle, char* buffer, size_t count, size_t offset, size_t* bytesRead)
{        
    int fd = open_by_handle_at(mFsEnv->mountfd, fileHandle.RawHandle(), O_RDWR);
    if (fd < 0) {
        LOG(ERROR) << "Failed to open  handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }
    *bytesRead = pread(fd, buffer, count, offset);
    close(fd);
    return butil::Status::OK();
}