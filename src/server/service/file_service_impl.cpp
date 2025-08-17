// src/server/service/file_service_impl.cpp
#include "file_service_impl.h"
#include <brpc/controller.h>
#include <brpc/closure_guard.h>
#include <glog/logging.h>

namespace ztofs {
namespace server {

FileServiceImpl::FileServiceImpl() {
    // Constructor implementation
    LOG(INFO) << "FileServiceImpl created";
}

void FileServiceImpl::Create(::google::protobuf::RpcController* controller,
                            const ::ztofs::CreateRequest* request,
                            ::ztofs::CreateResponse* response,
                            ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    
    LOG(INFO) << "Received CreateRequest for path: " << request->path();
    butil::Status status;
    /*
    // Create an instance of LocalStorage to handle the file operation
    LocalStorage storage;
    FileHandle fileHandle;
    
    // Call the storage Create method
    butil::Status status = storage.Create(request->path(), &fileHandle);
    */
    // Set response based on the operation result
    if (status.ok()) {
        response->set_status(0); // Success
        LOG(INFO) << "Successfully created file: " << request->path();
    } else {
        response->set_status(status.error_code());
        LOG(ERROR) << "Failed to create file: " << request->path() 
                   << ", error: " << status.error_str();
    }
}

void FileServiceImpl::Remove(::google::protobuf::RpcController* controller,
                            const ::ztofs::RemoveRequest* request,
                            ::ztofs::RemoveResponse* response,
                            ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    
    LOG(INFO) << "Received RemoveRequest for path: " << request->path();
    butil::Status status;
    /*
    // Create an instance of LocalStorage to handle the file operation
    LocalStorage storage;
    
    // For remove operation, we need a valid FileHandle
    // In a real implementation, you might want to check if file exists first
    FileHandle fileHandle(request->path(), -1);
    fileHandle.is_valid = true;
    
    // Call the storage Remove method
    butil::Status status = storage.Remove(fileHandle);
    */
    // Set response based on the operation result
    if (status.ok()) {
        response->set_status(0); // Success
        LOG(INFO) << "Successfully removed file: " << request->path();
    } else {
        response->set_status(status.error_code());
        LOG(ERROR) << "Failed to remove file: " << request->path() 
                   << ", error: " << status.error_str();
    }
}

} // namespace server
} // namespace ztofs