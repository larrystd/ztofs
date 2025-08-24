#pragma once

#include "meta.h"
namespace ztofs 
{
namespace server 
{

class LocalMeta : public MetaInterface
{
public:
    explicit LocalMeta(FileSystemEnv* fsenv) : mFsEnv(fsenv) {}

    butil::Status GetRootHandle(FileHandle* newHandle) override;

    butil::Status Create(const FileHandle& parenHandle, const std::string& name, FileTypePB type, FileHandle* newHandle) override;

    butil::Status Remove(const FileHandle& parenHandle, const std::string& name, FileTypePB type) override;

    butil::Status Lookup(const FileHandle& parenHandle, const std::string& name, FileHandle* newHandle) override;

    butil::Status GetAttr(const FileHandle& handle, FileAttr* attr) override;

    butil::Status SetAttr(const FileHandle& handle, const FileAttr& attr) override;

private:
    FileSystemEnv* mFsEnv{nullptr};
};

}
}