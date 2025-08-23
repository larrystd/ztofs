#pragma once

#include <string>
#include <butil/status.h>
#include "server/common/file_handle.h"
#include "server/common/file_attr.h"


namespace ztofs
{

namespace server
{

class MetaInterface
{
public:
    virtual butil::Status Create(const FileHandle& parenHandle, const std::string& name, FileHandle* newHandle) = 0;
    virtual butil::Status Remove(const FileHandle& parenHandle, const std::string& name) = 0;
    virtual butil::Status Lookup(const FileHandle& parenHandle, const std::string& name, FileHandle* newHandle) = 0;
    virtual butil::Status GetAttr(const FileHandle& handle, FileAttr* attr) = 0;
    virtual butil::Status SetAttr(const FileHandle& handle, const FileAttr& attr) = 0;

};

    
}
}