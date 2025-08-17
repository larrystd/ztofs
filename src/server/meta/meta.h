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
    virtual butil::Status Create(const std::string& path, FileHandle* fileHandle) = 0;
    virtual butil::Status Remove(const FileHandle& fileHandle) = 0;
};

    
}
}