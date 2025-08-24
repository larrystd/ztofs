#include "local_meta.h"
#include <fcntl.h>
#include "server/common/errorcode.h"
#include <butil/logging.h>
#include <sys/stat.h>

using namespace ztofs;
using namespace ztofs::server;

butil::Status LocalMeta::GetRootHandle(FileHandle* newHandle)
{
    int mountId;
    name_to_handle_at(mFsEnv->mountfd, "/", newHandle->RawHandle(), &mountId, 0);
    return butil::Status::OK();
}

butil::Status LocalMeta::Create(const FileHandle& parenHandle, const std::string& name, FileTypePB type, FileHandle* newHandle)
{
    int dir_fd = open_by_handle_at(mFsEnv->mountfd, parenHandle.RawHandle(), O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        LOG(ERROR) << "Failed to open parent handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }

    int new_fd;
    
    if (type == FileTypePB::DIRECTORY) {
        // For directories, use mkdirat
        if (mkdirat(dir_fd, name.c_str(), 0755) == -1) {
            close(dir_fd);
            LOG(ERROR) << "Failed to create directory: " << strerror(errno);
            return butil::Status(ZTO_CREATE_FAILED, "Failed to create directory: %s", strerror(errno));
        }
        
        // Open the newly created directory to get its handle
        new_fd = openat(dir_fd, name.c_str(), O_RDONLY | O_DIRECTORY);
    } else if (type == FileTypePB::FILE) {
        // For regular files, use openat
        new_fd = openat(dir_fd, name.c_str(), O_CREAT | O_RDWR | O_EXCL, 0644);
    } else {
        close(dir_fd);
        return butil::Status(EINVAL, "Invalid file type");
    }

    if (new_fd < 0) {
        close(dir_fd);
        LOG(ERROR) << "Failed to open parent handle:" << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }
    int mountId;
    name_to_handle_at(dir_fd, name.c_str(), newHandle->RawHandle(), &mountId, 0);

    close(new_fd);
    close(dir_fd);
    return butil::Status::OK();
}

butil::Status LocalMeta::Remove(const FileHandle& parenHandle, const std::string& name, FileTypePB type)
{   
    int dir_fd = open_by_handle_at(mFsEnv->mountfd, parenHandle.RawHandle(), O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        LOG(ERROR) << "Failed to open parent handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }
    int flags = 0;
    if (type == FileTypePB::DIRECTORY) {
        flags = AT_REMOVEDIR;  // For directories
    }
    if (unlinkat(dir_fd, name.c_str(), flags) == -1) {
        return butil::Status(errno, "Failed to remove file: %s", strerror(errno));
    }
    close(dir_fd);
    return butil::Status::OK();
}

butil::Status LocalMeta::Lookup(const FileHandle& parenHandle, const std::string& name,  FileHandle* newHandle)
{  
    int dir_fd = open_by_handle_at(mFsEnv->mountfd, parenHandle.RawHandle(), O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        LOG(ERROR) << "Failed to open parent handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }
    
    int mountId;
    name_to_handle_at(dir_fd, name.c_str(), newHandle->RawHandle(), &mountId, 0);
    close(dir_fd);
    return butil::Status::OK();
}

butil::Status LocalMeta::GetAttr(const FileHandle& fileHandle, FileAttr* attr)
{
    if (!attr) {
        return butil::Status(EINVAL, "Attribute pointer is null");
    }
    
    int fd = open_by_handle_at(mFsEnv->mountfd, fileHandle.RawHandle(), O_RDONLY);
    if (fd < 0) 
    {
        LOG(ERROR) << "Failed to open  handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open parent handle: %s", strerror(errno));
    }
    struct stat stat_buf;
    if (fstat(fd, &stat_buf) == -1) 
    {
        close(fd);
        LOG(ERROR) << "Failed to get file attributes: " << strerror(errno);
        return butil::Status(errno, "Failed to get file attributes: %s", strerror(errno));
    }
    attr->size = stat_buf.st_size;
    attr->atime = stat_buf.st_atim.tv_sec;
    attr->mtime = stat_buf.st_mtim.tv_sec;
    attr->ctime = stat_buf.st_ctim.tv_sec;
    attr->mode = stat_buf.st_mode;
    attr->uid = stat_buf.st_uid;
    attr->gid = stat_buf.st_gid;
    
    return butil::Status::OK();
}

butil::Status LocalMeta::SetAttr(const FileHandle& fileHandle, const FileAttr& attr)
{
    // Open file by handle to get file descriptor
    int fd = open_by_handle_at(mFsEnv->mountfd, fileHandle.RawHandle(), O_RDONLY);
    if (fd < 0) {
        LOG(ERROR) << "Failed to open handle: " << strerror(errno);
        return butil::Status(ZTO_CREATE_FAILED, "Failed to open handle: %s", strerror(errno));
    }
    
    // Change file mode/permissions using fchmod
    if (fchmod(fd, attr.mode) == -1) {
        close(fd);
        LOG(ERROR) << "Failed to change file mode: " << strerror(errno);
        return butil::Status(errno, "Failed to change file mode: %s", strerror(errno));
    }
    
    // Change file owner and group using fchown
    if (fchown(fd, attr.uid, attr.gid) == -1) {
        close(fd);
        LOG(ERROR) << "Failed to change file owner: " << strerror(errno);
        return butil::Status(errno, "Failed to change file owner: %s", strerror(errno));
    }
    
    // Change file access and modification times using futimens
    struct timespec times[2];
    times[0].tv_sec = attr.atime;  // Access time
    times[0].tv_nsec = 0;
    times[1].tv_sec = attr.mtime;  // Modification time
    times[1].tv_nsec = 0;
    
    if (futimens(fd, times) == -1) {
        close(fd);
        LOG(ERROR) << "Failed to change file times: " << strerror(errno);
        return butil::Status(errno, "Failed to change file times: %s", strerror(errno));
    }
    
    // Close the file descriptor
    close(fd);
    
    return butil::Status::OK();
}