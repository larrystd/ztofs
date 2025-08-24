#include <gflags/gflags.h>
#include <brpc/server.h>
#include <butil/logging.h>
#include <json2pb/pb_to_json.h>
#include <memory>
#include "meta/local_meta.h"
#include "storage/local_storage.h"
#include "server/service/echo_service.h"
#include "server/service/file_service_impl.h"

DEFINE_int32(port, 8006, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");

DEFINE_int32(thread_num, 8, "Number of threads to run");
DEFINE_int32(max_concurrency, 8, "Limit maximum number of concurrent calls");
// DEFINE_int32(event_dispatcher_num, 4, "Number of event dispatcher");
DEFINE_string(mount_path, "/", "fs mount path");

// ./bin/file_server --mount_path=/data --event_dispatcher_num=4

using namespace ztofs;

static std::unique_ptr<server::FileSystemEnv> gFsEnv;
static std::unique_ptr<server::MetaInterface> gLocalMeta;
static std::unique_ptr<server::StorageInterface> gLocalStorage;

int main(int argc, char* argv[]) {
    GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);

    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_FILE;
    settings.log_file = "file_server.log";
    settings.delete_old = logging::DELETE_OLD_LOG_FILE;
    InitLogging(settings);
    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.
    EchoServiceImpl echo_service_impl;

    if (server.AddService(&echo_service_impl, 
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add service";
        return -1;
    }
    
    gFsEnv = std::make_unique<server::FileSystemEnv>();
    gFsEnv->InitEnv(FLAGS_mount_path);
    gLocalMeta = std::make_unique<server::LocalMeta>(gFsEnv.get());
    gLocalStorage = std::make_unique<server::LocalStorage>(gFsEnv.get());
    LOG(INFO) << "root path: " << FLAGS_mount_path;
    LOG(INFO) << "thread_num: " << FLAGS_thread_num;

    ztofs::server::FileServiceImpl file_service_impl(gFsEnv.get(), gLocalMeta.get(), gLocalStorage.get());
    if (server.AddService(&file_service_impl, 
            brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add file service";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    options.num_threads = FLAGS_thread_num;
    options.max_concurrency = FLAGS_max_concurrency;
    if (server.Start(FLAGS_port, &options) != 0) {
        LOG(ERROR) << "Fail to start EchoServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}