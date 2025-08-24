#include <cstdint>
#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "butil/time/time.h"
#include "proto/file_service.pb.h"
#include <string>
#include "server/common/file_handle.h"
#include <iostream>

using namespace ztofs;
using namespace ztofs::server;

DEFINE_bool(send_attachment, true, "Carry attachment along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "0.0.0.0:8006", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(num_files, 5, "Number of files to create");
DEFINE_int32(file_size, 20, "filesize in MB");
DEFINE_int32(io_size, 1024, "iosize in KB");
DEFINE_int64(run_time_sec, 20, "run time in seconds");

int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);

    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_FILE;
    settings.log_file = "simple_benchmark.log";
    settings.delete_old = logging::DELETE_OLD_LOG_FILE;
    InitLogging(settings);

    std::unique_ptr<brpc::Channel> channel = std::make_unique<brpc::Channel>();

    // Initialize the channel, NULL means using default options.
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel->Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }

    ztofs::FileService_Stub stub(channel.get());
    // Test GetRootHandle
    GetRootHandleRequest request;
    GetRootHandleResponse response;
    
    std::unique_ptr<brpc::Controller> cntl = std::make_unique<brpc::Controller>();
    stub.GetRootHandle(cntl.get(), &request, &response, NULL);
    FileHandle rootHandle;
    rootHandle.FromPB(response.handle());

    std::string testDir = "testdir";
    CreateResponse* create_response = new CreateResponse();
    CreateRequest create_request;
    
    cntl.reset(new brpc::Controller());
    rootHandle.ToPB(create_request.mutable_parent_handle());
    create_request.set_name(testDir);
    create_request.set_type(FileTypePB::DIRECTORY);
    
    stub.Create(cntl.get(), &create_request, create_response, nullptr);
    FileHandlePB dirHandlePB = create_response->handle();

    uint64_t fileSize = FLAGS_file_size;
    uint32_t ioSize = FLAGS_io_size;
    butil::Time startTime = butil::Time::Now();
    // Loop to create and write files
    for (int i = 0; i < FLAGS_num_files; ++i) {
        // Create file
        std::unique_ptr<CreateResponse> create_response = std::make_unique<CreateResponse>();
        cntl.reset(new brpc::Controller());

        ztofs::CreateRequest create_request;
        
        std::string filename = "file_" + std::to_string(i) + ".txt";
        create_request.mutable_parent_handle()->CopyFrom(dirHandlePB);
        create_request.set_name(filename);
        
        stub.Create(cntl.get(), &create_request, create_response.get(), nullptr);
        
        if (cntl->Failed()) {
            LOG(ERROR) << "Failed to create file " << filename 
                       << ": " << cntl->ErrorText();
            continue;
        }
        
        LOG(INFO) << "Successfully created file: " << filename;
        
        uint64_t offset = 0;
        std::string content = std::string(1024*ioSize, 'a');
        while (offset < fileSize*1024*1024) {
            // Write to file
            std::unique_ptr<WriteResponse> write_response = std::make_unique<WriteResponse>();
            cntl.reset(new brpc::Controller());
            ztofs::WriteRequest write_request;
            write_request.mutable_handle()->CopyFrom(create_response->handle());
            write_request.set_offset(offset);
            write_request.set_data(content);
            
            stub.Write(cntl.get(), &write_request, write_response.get(), nullptr);
            
            if (cntl->Failed()) {
                LOG(ERROR) << "Failed to write to file " << filename 
                        << ": " << cntl->ErrorText();
            } else {
                LOG(INFO) << "Successfully wrote to file: " << filename 
                        << " with content: " << content;
            }
            offset += 1024*ioSize;
        }
    }
    uint64_t totalBytes = FLAGS_num_files * FLAGS_file_size * 1024 * 1024;
    uint64_t timeUs = (butil::Time::Now() - startTime).InMicroseconds();
    printf ("timeUs, %lu", timeUs);
    printf (" throughputMBps, %f\n", (totalBytes/1024.0/1024.0) / (timeUs / 1000000.0));
    printf ("unix ts, %ld\n", (butil::Time::Now().ToInternalValue()));

    // Loop to delete files
    for (int i = 0; i < FLAGS_num_files; ++i) {
        // Delete file
        std::unique_ptr<RemoveResponse> remove_response = std::make_unique<RemoveResponse>();
        cntl.reset(new brpc::Controller());
        
        ztofs::RemoveRequest remove_request;
        remove_request.mutable_parent_handle()->CopyFrom(dirHandlePB);
        std::string filename = "file_" + std::to_string(i) + ".txt";
        remove_request.set_name(filename);
        remove_request.set_type(FileTypePB::FILE);
        
        stub.Remove(cntl.get(), &remove_request, remove_response.get(), nullptr);
        
        if (cntl->Failed()) {
            LOG(ERROR) << "Failed to delete file " << filename 
                    << ": " << cntl->ErrorText();
        } else {
            LOG(INFO) << "Successfully deleted file: " << filename;
        }
    }

    ztofs::RemoveRequest remove_request;
    std::unique_ptr<RemoveResponse> remove_response = std::make_unique<RemoveResponse>();

    cntl.reset(new brpc::Controller());
    rootHandle.ToPB(remove_request.mutable_parent_handle());
    remove_request.set_name(testDir);
    remove_request.set_type(FileTypePB::DIRECTORY);
    stub.Remove(cntl.get(), &remove_request, remove_response.get(), nullptr);

    LOG(INFO) << "Final file client request completed";
    return 0;
}