#pragma once

#include <string>
#include <butil/status.h>
#include "server/common/file_handle.h"

namespace ztofs
{

namespace server
{

class MetaInterface
{
public:
    virtual butil::Status Create(const FileHandle& parenHandle, const std::string& name, FileHandle* newHandle) = 0;
    virtual butil::Status Remove(const FileHandle& parenHandle, const std::string& name) = 0;
    virtual butil::Status Open(const FileHandle& handle, int flags) = 0;
};

    
}
}