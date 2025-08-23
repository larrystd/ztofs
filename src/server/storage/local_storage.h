#pragma once

#include "storage.h"
#include "server/common/file_handle.h"
#include "server/common/errorcode.h"

namespace ztofs 
{
namespace server 
{

class LocalStorage : public StorageInterface
{
public:
    explicit LocalStorage(FileSystemEnv* fsenv) : mFsEnv(fsenv) {}

    butil::Status Write(const FileHandle& fileHandle, const char* buffer, size_t count, size_t offset, size_t* bytesWritten) override;
    butil::Status Read(const FileHandle& fileHandle, char* buffer, size_t count, size_t offset, size_t* bytesRead) override;

private:
    FileSystemEnv* mFsEnv{nullptr};
};
    
}
}