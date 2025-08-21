#pragma once

#include <string>
#include <butil/status.h>
#include "server/common/file_handle.h"

namespace ztofs
{

namespace server
{

class StorageInterface
{
public:
    virtual butil::Status Write(const FileHandle& fileHandle, const char* buffer, size_t count, size_t* bytesWritten) = 0;
    virtual butil::Status Read(const FileHandle& fileHandle, char* buffer, size_t count, size_t* bytesRead) = 0;
};

    
}
}