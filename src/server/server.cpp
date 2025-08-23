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

using namespace ztofs;

static std::string gMountPath = "/";
static server::FileSystemEnv gFsEnv;
static std::unique_ptr<server::MetaInterface> gLocalMeta;
static std::unique_ptr<server::StorageInterface> gLocalStorage;

int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);

    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.
    EchoServiceImpl echo_service_impl;

    if (server.AddService(&echo_service_impl, 
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add service";
        return -1;
    }

    gFsEnv.InitEnv(gMountPath);
    gLocalMeta = std::make_unique<server::LocalMeta>(&gFsEnv);
    gLocalStorage = std::make_unique<server::LocalStorage>(&gFsEnv);

    ztofs::server::FileServiceImpl file_service_impl(gLocalMeta.get(), gLocalStorage.get());
    if (server.AddService(&file_service_impl, 
            brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add file service";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
        LOG(ERROR) << "Fail to start EchoServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}