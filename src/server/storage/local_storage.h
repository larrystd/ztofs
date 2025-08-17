#pragma once

#include "storage.h"

namespace ztofs 
{
namespace server 
{

class LocalStorage : public StorageInterface
{
public:
    butil::Status Write(FileHandle* fileHandle, const char* buffer, size_t count) override;
    butil::Status Read(FileHandle* fileHandle, char* buffer, size_t count, size_t* bytesRead) override;
};
    
}
}