#include "local_meta.h"
#include <fcntl.h>
#include "server/common/errorcode.h"
#include <butil/logging.h>

using namespace ztofs;
using namespace ztofs::server;

butil::Status LocalMeta::Create(const FileHandle& parenHandle, const std::string& name, FileHandle* newHandle)
{
    int dir_fd = open_by_handle_at(mFsEnv->mountfd, parenHandle.rawhandle.get(), O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        LOG(ERROR) << "Failed to open parent handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }

    int new_fd = openat(dir_fd, name.c_str(), O_CREAT | O_RDWR | O_EXCL, 0644);
    
    if (new_fd < 0) {
        close(dir_fd);
        LOG(ERROR) << "Failed to open parent handle:" << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }
    int mountId;
    name_to_handle_at(dir_fd, name.c_str(), newHandle->rawhandle.get(), &mountId, 0);

    close(new_fd);
    close(dir_fd);
    return butil::Status::OK();
}

butil::Status LocalMeta::Remove(const FileHandle& parenHandle, const std::string& name)
{   
    int dir_fd = open_by_handle_at(mFsEnv->mountfd, parenHandle.rawhandle.get(), O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        LOG(ERROR) << "Failed to open parent handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }
    if (unlinkat(dir_fd, name.c_str(), 0) == -1) {
        return butil::Status(errno, "Failed to remove file: %s", strerror(errno));
    }
    close(dir_fd);
    return butil::Status::OK();
}

butil::Status LocalMeta::Open(const FileHandle& handle, int flags)
{    
    int fd = open_by_handle_at(mFsEnv->mountfd, handle.rawhandle.get(), flags);
    if (fd == -1) {
        return butil::Status(errno, "Failed to open file by handle: %s", strerror(errno));
    }
    return butil::Status::OK();
}