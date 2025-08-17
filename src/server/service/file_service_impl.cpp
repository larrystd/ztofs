// src/server/service/file_service_impl.cpp
#include "file_service_impl.h"
#include <brpc/controller.h>
#include <brpc/closure_guard.h>
#include <butil/logging.h>

namespace ztofs {
namespace server {

void FileServiceImpl::Create(::google::protobuf::RpcController* controller,
                            const ::ztofs::CreateRequest* request,
                            ::ztofs::CreateResponse* response,
                            ::google::protobuf::Closure* done) 
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    
    LOG(INFO) << "Received CreateRequest for path: " << request->path();

    FileHandle fileHandle;
    butil::Status status = meta_->Create(request->path(), &fileHandle);

    if (status.ok()) {
        response->set_status(0); // Success
        LOG(INFO) << "Successfully created file: " << request->path();
        return;
    }
    response->set_status(status.error_code());
        LOG(ERROR) << "Failed to create file: " << request->path() 
                   << ", error: " << status.error_str();
}

void FileServiceImpl::Remove(::google::protobuf::RpcController* controller,
                            const ::ztofs::RemoveRequest* request,
                            ::ztofs::RemoveResponse* response,
                            ::google::protobuf::Closure* done) 
{
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    
    LOG(INFO) << "Received RemoveRequest for path: " << request->path();

    FileHandle fileHandle(request->path(), -1);
    butil::Status status = meta_->Remove(fileHandle);
    // Set response based on the operation result
    if (status.ok()) {
        response->set_status(0); // Success
        LOG(INFO) << "Successfully removed file: " << request->path();
        return;
    }
    response->set_status(status.error_code());
    LOG(ERROR) << "Failed to create file: " << request->path() 
               << ", error: " << status.error_str();
}

} // namespace server
} // namespace ztofs