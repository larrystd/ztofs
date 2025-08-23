#pragma once

#include <fcntl.h>
#include <memory>
#include <string>
#include <unistd.h>
#include "proto/file_common.pb.h"

namespace ztofs
{
namespace server
{

class FileHandle
{
public:
    FileHandle() {
        mRawhandle = std::make_unique<struct file_handle>();
        mRawhandle->handle_bytes = 128;
    }
    FileHandle(struct file_handle* handle)
    {
        mRawhandle.reset(handle); 
    }

    struct file_handle* RawHandle() const {
        return mRawhandle.get();
    }
    
    FileHandle& operator=(FileHandle&& other)
    {
        if (this != &other) {
            mRawhandle = std::move(other.mRawhandle);
        }
        return *this;
    }
    
    void ToPB(FileHandlePB* pb) {
        if (!pb) {
            return;
        }
        pb->set_handle_data(mRawhandle->f_handle, mRawhandle->handle_bytes);
        pb->set_handle_type(mRawhandle->handle_type);
    }

    void FromPB(const FileHandlePB& pb) {
        size_t len = pb.handle_data().size();
        if (len > 128) {
            len = 128;
        }
        mRawhandle->handle_bytes = len;
        memcpy(mRawhandle->f_handle, pb.handle_data().data(), len);
        mRawhandle->handle_type = pb.handle_type();
    }

private:
    std::unique_ptr<struct file_handle> mRawhandle;
};

class FileSystemEnv
{
public:
    int mountfd{-1};
    std::string mountpath;

    void InitEnv(const std::string& rootPath)
    {
        mountpath = rootPath;
        mountfd = open(mountpath.c_str(), O_RDONLY | O_DIRECTORY);
        if (mountfd < 0) {
            perror("open mount path failed");
            exit(1);
        }
    }

    ~FileSystemEnv()
    {
        if (mountfd >= 0) {
            close(mountfd);
        }
    }
};

}
}