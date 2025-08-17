#pragma once

#include "meta.h"

namespace ztofs 
{
namespace server 
{

class LocalMeta : public MetaInterface
{
public:
    butil::Status Create(const std::string& path, FileHandle* fileHandle) override;
    butil::Status Remove(const FileHandle& fileHandle) override;
    butil::Status Open(const std::string& path, FileHandle* fileHandle) override;
};

}
}