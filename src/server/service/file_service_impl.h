// src/server/service/storage_service.h
#pragma once

#include "proto/file_service.pb.h"
#include <brpc/server.h>

namespace ztofs {
namespace server {

class FileServiceImpl : public FileService {
public:
FileServiceImpl();
    virtual ~FileServiceImpl() = default;
    
    void Create(::google::protobuf::RpcController* controller,
                const ::ztofs::CreateRequest* request,
                ::ztofs::CreateResponse* response,
                ::google::protobuf::Closure* done) override;
    
    void Remove(::google::protobuf::RpcController* controller,
                const ::ztofs::RemoveRequest* request,
                ::ztofs::RemoveResponse* response,
                ::google::protobuf::Closure* done) override;
};

} // namespace server
} // namespace ztofs