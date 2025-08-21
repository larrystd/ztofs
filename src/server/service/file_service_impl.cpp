// src/server/service/file_service_impl.cpp
#include "file_service_impl.h"
#include <brpc/controller.h>
#include <brpc/closure_guard.h>
#include <butil/logging.h>
#include "server/common/file_handle.h"

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

    // 解析路径，分离目录和文件名
    std::string path = request->path();
    size_t pos = path.find_last_of('/');
    std::string dir_path = (pos == 0) ? "/" : path.substr(0, pos);
    std::string file_name = path.substr(pos + 1);

    // 创建父目录句柄和新文件句柄
    FileHandle parentHandle;
    FileHandle newHandle;
    
    // 调用元数据接口创建文件
    butil::Status status = meta_->Create(parentHandle, file_name, &newHandle);

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

    // 解析路径，分离目录和文件名
    std::string path = request->path();
    size_t pos = path.find_last_of('/');
    std::string dir_path = (pos == 0) ? "/" : path.substr(0, pos);
    std::string file_name = path.substr(pos + 1);

    // 创建父目录句柄
    FileHandle parentHandle;
    
    // 调用元数据接口删除文件
    butil::Status status = meta_->Remove(parentHandle, file_name);
    
    // Set response based on the operation result
    if (status.ok()) {
        response->set_status(0); // Success
        LOG(INFO) << "Successfully removed file: " << request->path();
        return;
    }
    response->set_status(status.error_code());
    LOG(ERROR) << "Failed to remove file: " << request->path() 
               << ", error: " << status.error_str();
}

} // namespace server
} // namespace ztofs