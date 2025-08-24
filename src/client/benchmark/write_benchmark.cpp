#include <cstdint>
#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "butil/time/time.h"
#include "proto/file_service.pb.h"
#include <string>
#include "server/common/errorcode.h"
#include "server/common/file_handle.h"
#include <iostream>
#include "butil/rand_util.h"

using namespace ztofs;
using namespace ztofs::server;

DEFINE_bool(send_attachment, true, "Carry attachment along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "0.0.0.0:8006", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(num_files, 20, "Number of files to create");
DEFINE_int32(file_size, 1024, "filesize in MB");
DEFINE_int32(io_size, 1024, "iosize in KB");
DEFINE_bool(seq_io, true, "whether to use sequential or random IO");
DEFINE_int64(run_time_sec, 2, "run time in seconds");
DEFINE_int64(bthread_count, 8, "run time in seconds");

uint64_t getNextOffset(uint64_t oldOffset, int64_t ioSize, int64_t fileSize, bool seq)
{
    if (seq) {
        int64_t offset = oldOffset + ioSize;
        if (offset >= fileSize) {
            offset = 0;
        }
        return offset;
    } else {
        return butil::RandInt(0, fileSize);
    }
    return 0;
}

struct ThreadParams {
    // input
    int threadIdx;
    FileHandlePB dirHandlePB;
    // output 
    uint32_t iops;
    uint64_t throughput;
};
void* threadRunWrite(void* args)
{
    ThreadParams* threadArgs = (ThreadParams*)args;
    int threadIdx = threadArgs->threadIdx;
    LOG(INFO) << "thread " << threadIdx << " start";
    FileHandlePB dirHandlePB = threadArgs->dirHandlePB;
    std::unique_ptr<brpc::Channel> channel = std::make_unique<brpc::Channel>();

    // Initialize the channel, NULL means using default options.
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel->Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return NULL;
    }

    ztofs::FileService_Stub stub(channel.get());
    
    std::unique_ptr<brpc::Controller> cntl = std::make_unique<brpc::Controller>();

    uint64_t fileSize = FLAGS_file_size * 1024 * 1024;
    uint32_t ioSize = FLAGS_io_size * 1024;
    std::string content = std::string(ioSize, 'a');

    butil::Time current = butil::Time::Now();
    butil::Time startTime = current;
    butil::Time endTime = current + butil::TimeDelta::FromSeconds(FLAGS_run_time_sec);
    uint32_t fileIdx = 0;

    // Create file
    current = butil::Time::Now();
    std::unique_ptr<CreateResponse> create_response = std::make_unique<CreateResponse>();
    cntl.reset(new brpc::Controller());

    ztofs::CreateRequest create_request;
    
    std::string filename = std::to_string(threadIdx) + "_file_" + std::to_string(fileIdx) + ".txt";
    create_request.mutable_parent_handle()->CopyFrom(dirHandlePB);
    create_request.set_name(filename);
    create_request.set_type(::ztofs::FileTypePB::FILE);
    
    stub.Create(cntl.get(), &create_request, create_response.get(), nullptr);
    
    if (cntl->Failed()) {
        LOG(ERROR) << "Failed to create file " << filename 
                    << ": " << cntl->ErrorText();
        return NULL;
    }
        
    LOG(INFO) << "Successfully created file: " << filename;
        
    std::unique_ptr<WriteResponse> write_response = std::make_unique<WriteResponse>();
    ztofs::WriteRequest write_request;

    uint32_t offset = 0;
    uint32_t writtenBytes = 0;
    uint32_t ioTimes = 0;
    while (true) 
    {
        // Write to file
        current = butil::Time::Now();
        if (current >= endTime) {
            break;
        }
        cntl.reset(new brpc::Controller());
        write_request.mutable_handle()->CopyFrom(create_response->handle());
        write_request.set_offset(offset);
        write_request.set_data(content);
        
        stub.Write(cntl.get(), &write_request, write_response.get(), nullptr);
        
        if (cntl->Failed()) {
            LOG(ERROR) << "Failed to write to file " << filename 
                    << ": " << cntl->ErrorText();
        }
        if (write_response->status() == ZTO_OK)
        {
            writtenBytes += write_response->bytes_written();
            ioTimes++;
        }
        offset = getNextOffset(offset, ioSize, fileSize, FLAGS_seq_io);
        // printf("offset: %u\n", offset);
    }

    uint64_t totalBytes = writtenBytes;
    uint64_t timeUs = (butil::Time::Now() - startTime).InMicroseconds();
    uint64_t throughput = totalBytes/ (timeUs / 1000000.0);
    printf ("timeUs, %lu", timeUs);
    printf (" throughputMBps, %f\n", (totalBytes/1024.0/1024.0) / (timeUs / 1000000.0));
    
    threadArgs->throughput = throughput;
    threadArgs->iops = ioTimes / (timeUs / 1000000.0);
    return NULL;
}



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

    std::vector<bthread_t> bids;
    bids.resize(FLAGS_bthread_count);

    
    std::unique_ptr<ThreadParams[]> threadArgs = std::make_unique<ThreadParams[]>(FLAGS_bthread_count);
    for (int i = 0; i < FLAGS_bthread_count; ++i) {
        threadArgs[i].threadIdx = i;
        threadArgs[i].dirHandlePB = dirHandlePB;
        if (bthread_start_background(
                &bids[i], NULL, threadRunWrite, &threadArgs[i]) != 0) {
            LOG(ERROR) << "Fail to create bthread";
            return -1;
        }
    }
    // threadRunWrite(0, dirHandlePB);

    for (int i = 0; i < FLAGS_bthread_count; ++i) {
        bthread_join(bids[i], NULL);
    }
    uint64_t throughput = 0;
    uint64_t iops = 0;
    for (int i = 0; i < FLAGS_bthread_count; ++i) {
        throughput += threadArgs[i].throughput;
        iops += threadArgs[i].iops;
    }
    printf ("Total throughputMBps, %f\n", throughput/1024.0/1024.0);
    printf ("Total iops, %ld\n", iops);
    LOG(INFO) << "Final file client request completed";
    return 0;
}