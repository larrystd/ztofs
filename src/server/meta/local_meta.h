#pragma once

#include "meta.h"

namespace ztofs 
{
namespace server 
{

class LocalMeta : public MetaInterface
{
public:
    LocalMeta() {}
    explicit LocalMeta(FileSystemEnv* fsenv) : mFsEnv(fsenv) {}

    butil::Status Create(const FileHandle& parenHandle, const std::string& name, FileHandle* newHandle) override;

    butil::Status Remove(const FileHandle& parenHandle, const std::string& name) override;

    butil::Status Open(const FileHandle& handle, int flags) override;

private:
    FileSystemEnv* mFsEnv{nullptr};
};

}
}