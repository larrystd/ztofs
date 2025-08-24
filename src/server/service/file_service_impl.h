// src/server/service/storage_service.h
#pragma once

#include "proto/file_service.pb.h"
#include <brpc/server.h>
#include "server/common/file_handle.h"
#include "server/meta/meta.h"
#include "server/storage/storage.h"

namespace ztofs {
namespace server {

class FileServiceImpl : public FileService {
public:
    FileServiceImpl(FileSystemEnv* fsEnv, MetaInterface* meta, StorageInterface* storage)
        : mFsEnv(fsEnv),
          mMeta(meta),
          mStorage(storage)
    {}
    virtual ~FileServiceImpl() = default;

    void GetRootHandle(::google::protobuf::RpcController* controller,
        const ::ztofs::GetRootHandleRequest* request,
        ::ztofs::GetRootHandleResponse* response,
        ::google::protobuf::Closure* done) override;
    
    void Create(::google::protobuf::RpcController* controller,
                const ::ztofs::CreateRequest* request,
                ::ztofs::CreateResponse* response,
                ::google::protobuf::Closure* done) override;
    
    void Remove(::google::protobuf::RpcController* controller,
                const ::ztofs::RemoveRequest* request,
                ::ztofs::RemoveResponse* response,
                ::google::protobuf::Closure* done) override;

    void Lookup(::google::protobuf::RpcController* controller,
        const ::ztofs::LookupRequest* request,
        ::ztofs::LookupResponse* response,
        ::google::protobuf::Closure* done) override;

    void GetAttr(::google::protobuf::RpcController* controller,
        const ::ztofs::GetAttrRequest* request,
        ::ztofs::GetAttrResponse* response,
        ::google::protobuf::Closure* done) override;

    void SetAttr(::google::protobuf::RpcController* controller,
        const ::ztofs::SetAttrRequest* request,
        ::ztofs::SetAttrResponse* response,
        ::google::protobuf::Closure* done) override;

    void Read(::google::protobuf::RpcController* controller,
        const ::ztofs::ReadRequest* request,
        ::ztofs::ReadResponse* response,
        ::google::protobuf::Closure* done) override;

    void Write(::google::protobuf::RpcController* controller,
        const ::ztofs::WriteRequest* request,
        ::ztofs::WriteResponse* response,
        ::google::protobuf::Closure* done) override;

private:
    FileSystemEnv* mFsEnv{nullptr};
    MetaInterface* mMeta{nullptr};
    StorageInterface *mStorage{nullptr};
};

} // namespace server
} // namespace ztofs