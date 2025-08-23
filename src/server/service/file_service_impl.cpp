// src/server/service/file_service_impl.cpp
#include "file_service_impl.h"
#include <brpc/controller.h>
#include <brpc/closure_guard.h>
#include <butil/logging.h>
#include "server/common/file_handle.h"

namespace ztofs {
namespace server {

void FileServiceImpl::Create(::google::protobuf::RpcController* controller,
        const ::ztofs::CreateRequest*request,
        ::ztofs::CreateResponse* response,
        ::google::protobuf::Closure* done) 
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

    FileHandle parentHandle;
    parentHandle.FromPB(request->parent_handle());

    std::string name = request->name();
    std::unique_ptr<FileHandle> newHandle = std::make_unique<FileHandle>();
    auto status = mMeta->Create(parentHandle, name, newHandle.get());
    if (status.ok()) {
        newHandle->ToPB(response->mutable_handle());
    } else {
        cntl->SetFailed(status.error_code(), "%s", status.error_cstr());
    }
    response->set_status(status.error_code());
}

void FileServiceImpl::Remove(::google::protobuf::RpcController* controller,
    const ::ztofs::RemoveRequest* request,
    ::ztofs::RemoveResponse* response,
    ::google::protobuf::Closure* done)
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

    FileHandle parentHandle;
    parentHandle.FromPB(request->parent_handle());
    std::string name = request->name();

    auto status = mMeta->Remove(parentHandle, name);
    if (!status.ok()) {
        cntl->SetFailed(status.error_code(), "%s", status.error_cstr());
    }
    response->set_status(status.error_code());
}

void FileServiceImpl::Lookup(::google::protobuf::RpcController* controller,
    const ::ztofs::LookupRequest* request,
    ::ztofs::LookupResponse* response,
    ::google::protobuf::Closure* done)
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

    FileHandle parentHandle;
    parentHandle.FromPB(request->parent_handle());

    std::string name = request->name();
    std::unique_ptr<FileHandle> foundHandle = std::make_unique<FileHandle>();

    auto status = mMeta->Lookup(parentHandle, name, foundHandle.get());
    if (status.ok()) {
        foundHandle->ToPB(response->mutable_handle());
    } else {
        cntl->SetFailed(status.error_code(), "%s", status.error_cstr());
    }
    response->set_status(status.error_code());
}

void FileServiceImpl::GetAttr(::google::protobuf::RpcController* controller,
    const ::ztofs::GetAttrRequest* request,
    ::ztofs::GetAttrResponse* response,
    ::google::protobuf::Closure* done)
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

    FileHandle handle;
    handle.FromPB(request->handle());

    FileAttr attr;
    auto status = mMeta->GetAttr(handle, &attr);
    if (status.ok()) {
        attr.ToPB(response->mutable_attr());
    } else {
        cntl->SetFailed(status.error_code(), "%s", status.error_cstr());
    }
    response->set_status(status.error_code());
}

void FileServiceImpl::SetAttr(::google::protobuf::RpcController* controller,
    const ::ztofs::SetAttrRequest* request,
    ::ztofs::SetAttrResponse* response,
    ::google::protobuf::Closure* done)
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

    FileHandle handle;
    handle.FromPB(request->handle());

    FileAttr attr;
    attr.FromPB(request->attr());

    auto status = mMeta->SetAttr(handle, attr);
    if (!status.ok()) {
        cntl->SetFailed(status.error_code(), "%s", status.error_cstr());
    }
    response->set_status(status.error_code());
}

void FileServiceImpl::Read(::google::protobuf::RpcController* controller,
    const ::ztofs::ReadRequest* request,
    ::ztofs::ReadResponse* response,
    ::google::protobuf::Closure* done)
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

    FileHandle handle;
    handle.FromPB(request->handle());

    off_t offset = request->offset();
    size_t size = request->size();

    // Allocate buffer for reading data
    std::unique_ptr<char[]> buffer(new char[size]);
    size_t bytesRead = 0;
    auto status = mStorage->Read(handle, buffer.get(), size, offset, &bytesRead);
    if (status.ok()) {
        response->set_data(buffer.get(), bytesRead);
    } else {
        cntl->SetFailed(status.error_code(), "%s", status.error_cstr());
    }
    response->set_status(status.error_code());
}

void FileServiceImpl::Write(::google::protobuf::RpcController* controller,
    const ::ztofs::WriteRequest* request,
    ::ztofs::WriteResponse* response,
    ::google::protobuf::Closure* done)
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

    FileHandle handle;
    handle.FromPB(request->handle());

    off_t offset = request->offset();
    std::string data = request->data();
    
    size_t bytesWritten = 0;
    auto status = mStorage->Write(handle, data.c_str(), data.size(), offset, &bytesWritten);
    if (status.ok()) {
        response->set_bytes_written(bytesWritten);
    } else {
        cntl->SetFailed(status.error_code(), "%s", status.error_cstr());
    }
    response->set_status(status.error_code());
}

} // namespace server
} // namespace ztofs