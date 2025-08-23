#include <gtest/gtest.h>
#include <brpc/server.h>
#include <brpc/channel.h>
#include "server/service/file_service_impl.h"
#include "server/meta/local_meta.h"
#include "server/storage/local_storage.h"
#include "proto/file_service.pb.h"
#include <memory>
#include <butil/file_util.h>
#include <fcntl.h>

using namespace ztofs;
using namespace ztofs::server;

class FileServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test directory
        test_dir_ = "/tmp/ztofs_test";
        mkdir(test_dir_.c_str(), 0755);
        
        // Initialize file system environment=
        fs_env_.InitEnv("/");
        
        // Create meta layer
        meta_ = std::make_unique<LocalMeta>(&fs_env_);
        storage_ = std::make_unique<LocalStorage>(&fs_env_);
        
        // Create service implementation
        service_ = std::make_unique<FileServiceImpl>(meta_.get(), storage_.get());
        
        // Set up server
        server_ = std::make_unique<brpc::Server>();
        if (server_->AddService(service_.get(), brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
            FAIL() << "Fail to add service";
        }
        
        brpc::ServerOptions options;
        options.idle_timeout_sec = -1;
        if (server_->Start("127.0.0.1:9999", &options) != 0) {
            FAIL() << "Fail to start server";
        }
        
        // Set up channel
        brpc::ChannelOptions channel_options;
        channel_options.timeout_ms = 1000;
        channel_.Init("127.0.0.1:9999", &channel_options);
    }
    
    void TearDown() override {
        server_->Stop(0);
        server_->Join();
    
        // Clean up test directory
        butil::DeleteFile(butil::FilePath(test_dir_), true);
    }
    
    std::string test_dir_;
    FileSystemEnv fs_env_;
    std::unique_ptr<LocalMeta> meta_;
    std::unique_ptr<LocalStorage> storage_;
    std::unique_ptr<FileServiceImpl> service_;
    std::unique_ptr<brpc::Server> server_;
    brpc::Channel channel_;
};

TEST_F(FileServiceTest, CreateAndRemoveFile) {
    FileService_Stub stub(&channel_);
    
    // First, create a test directory handle
    FileHandle parentHandle;
    int mount_id;
    ASSERT_GE(name_to_handle_at(AT_FDCWD, test_dir_.c_str(), parentHandle.RawHandle(), &mount_id, 0), 0);
    
    // Test Create
    CreateRequest create_request;
    CreateResponse create_response;
    FileHandlePB* parent_pb = create_request.mutable_parent_handle();
    parentHandle.ToPB(parent_pb);
    create_request.set_name("test_file");
    
    brpc::Controller create_cntl;
    stub.Create(&create_cntl, &create_request, &create_response, NULL);
    
    ASSERT_FALSE(create_cntl.Failed()) << "Create failed: " << create_cntl.ErrorText();
    ASSERT_TRUE(create_response.has_handle()) << "Create response should contain handle";
    
    // Test Lookup
    LookupRequest lookup_request;
    LookupResponse lookup_response;
    lookup_request.mutable_parent_handle()->CopyFrom(create_request.parent_handle());
    lookup_request.set_name("lookup_test_file");
    
    brpc::Controller lookup_cntl;
    stub.Lookup(&lookup_cntl, &lookup_request, &lookup_response, NULL);
    
    ASSERT_FALSE(lookup_cntl.Failed()) << "Lookup failed: " << lookup_cntl.ErrorText();
    ASSERT_TRUE(lookup_response.has_handle()) << "Lookup response should contain handle";
    ASSERT_EQ(lookup_response.status(), 0) << "Lookup should return success status";

    // Test Remove
    RemoveRequest remove_request;
    RemoveResponse remove_response;
    remove_request.mutable_parent_handle()->CopyFrom(create_request.parent_handle());
    remove_request.set_name("test_file");
    
    brpc::Controller remove_cntl;
    stub.Remove(&remove_cntl, &remove_request, &remove_response, NULL);
    
    ASSERT_FALSE(remove_cntl.Failed()) << "Remove failed: " << remove_cntl.ErrorText();
}

TEST_F(FileServiceTest, GetAndSetAttr) {
    FileService_Stub stub(&channel_);
    
    // First, create a test directory handle
    FileHandle parentHandle;
    int mount_id;
    ASSERT_GE(name_to_handle_at(AT_FDCWD, test_dir_.c_str(), parentHandle.RawHandle(), &mount_id, 0), 0);
    
    // Create a test file first
    CreateRequest create_request;
    CreateResponse create_response;
    FileHandlePB* parent_pb = create_request.mutable_parent_handle();
    parentHandle.ToPB(parent_pb);
    create_request.set_name("setattr_test_file");
    
    brpc::Controller create_cntl;
    stub.Create(&create_cntl, &create_request, &create_response, NULL);
    
    ASSERT_FALSE(create_cntl.Failed()) << "Create failed: " << create_cntl.ErrorText();
    ASSERT_TRUE(create_response.has_handle()) << "Create response should contain handle";
    ASSERT_EQ(create_response.status(), 0) << "Create should return success status";
    
    // Test GetAttr on the newly created file
    GetAttrRequest getattr_request;
    GetAttrResponse getattr_response;
    getattr_request.mutable_handle()->CopyFrom(create_response.handle());
    
    brpc::Controller getattr_cntl;
    stub.GetAttr(&getattr_cntl, &getattr_request, &getattr_response, NULL);
    
    ASSERT_FALSE(getattr_cntl.Failed()) << "GetAttr failed: " << getattr_cntl.ErrorText();
    ASSERT_EQ(getattr_response.status(), 0) << "GetAttr should return success status";
    ASSERT_TRUE(getattr_response.has_attr()) << "GetAttr response should contain attributes";
    
    // Store initial attributes for reference
    auto initial_attr = getattr_response.attr();
    
    // Test SetAttr to modify file attributes
    SetAttrRequest setattr_request;
    SetAttrResponse setattr_response;
    setattr_request.mutable_handle()->CopyFrom(create_response.handle());
    
    // Modify some attributes - change mode and set specific times
    auto* attr = setattr_request.mutable_attr();
    attr->set_mode((initial_attr.mode() & ~0777) | 0644);  // Change permissions to 644
    
    brpc::Controller setattr_cntl;
    stub.SetAttr(&setattr_cntl, &setattr_request, &setattr_response, NULL);
    
    ASSERT_FALSE(setattr_cntl.Failed()) << "SetAttr failed: " << setattr_cntl.ErrorText();
    ASSERT_EQ(setattr_response.status(), 0) << "SetAttr should return success status";
   
}

TEST_F(FileServiceTest, ReadWrite) {
    FileService_Stub stub(&channel_);
    
    // First, create a test directory handle
    FileHandle parentHandle;
    int mount_id;
    ASSERT_GE(name_to_handle_at(AT_FDCWD, test_dir_.c_str(), parentHandle.RawHandle(), &mount_id, 0), 0);
    
    // Create a test file first
    CreateRequest create_request;
    CreateResponse create_response;
    FileHandlePB* parent_pb = create_request.mutable_parent_handle();
    parentHandle.ToPB(parent_pb);
    create_request.set_name("readwrite_test_file");
    
    brpc::Controller create_cntl;
    stub.Create(&create_cntl, &create_request, &create_response, NULL);
    
    ASSERT_FALSE(create_cntl.Failed()) << "Create failed: " << create_cntl.ErrorText();
    ASSERT_TRUE(create_response.has_handle()) << "Create response should contain handle";
    ASSERT_EQ(create_response.status(), 0) << "Create should return success status";
    
    // Test Write to the file
    WriteRequest write_request;
    WriteResponse write_response;
    write_request.mutable_handle()->CopyFrom(create_response.handle());
    write_request.set_offset(0);
    std::string test_data = "Hello, ZTOFS!";
    write_request.set_data(test_data);
    
    brpc::Controller write_cntl;
    stub.Write(&write_cntl, &write_request, &write_response, NULL);
    
    ASSERT_FALSE(write_cntl.Failed()) << "Write failed: " << write_cntl.ErrorText();
    ASSERT_EQ(write_response.status(), 0) << "Write should return success status";
    ASSERT_EQ(write_response.bytes_written(), test_data.size()) << "Should write all bytes";

    // Test Read from the file
    ReadRequest read_request;
    ReadResponse read_response;
    read_request.mutable_handle()->CopyFrom(create_response.handle());
    read_request.set_offset(0);
    read_request.set_size(test_data.size());
    
    brpc::Controller read_cntl;
    stub.Read(&read_cntl, &read_request, &read_response, NULL);
    
    ASSERT_FALSE(read_cntl.Failed()) << "Read failed: " << read_cntl.ErrorText();
    ASSERT_EQ(read_response.status(), 0) << "Read should return success status";
    ASSERT_TRUE(read_response.has_data()) << "Read response should contain data";
    ASSERT_EQ(read_response.data(), test_data) << "Read data should match written data";
    printf("data %s\n", read_response.data().c_str());
    
    // Test partial read
    ReadRequest partial_read_request;
    ReadResponse partial_read_response;
    partial_read_request.mutable_handle()->CopyFrom(create_response.handle());
    partial_read_request.set_offset(7);  // Start from "ZTOFS!"
    partial_read_request.set_size(5);    // Read 5 bytes
    
    brpc::Controller partial_read_cntl;
    stub.Read(&partial_read_cntl, &partial_read_request, &partial_read_response, NULL);
    
    ASSERT_FALSE(partial_read_cntl.Failed()) << "Partial read failed: " << partial_read_cntl.ErrorText();
    ASSERT_EQ(partial_read_response.status(), 0) << "Partial read should return success status";
    ASSERT_TRUE(partial_read_response.has_data()) << "Partial read response should contain data";
    ASSERT_EQ(partial_read_response.data(), "ZTOFS") << "Partial read data should match";
    
    // Test write at offset
    WriteRequest offset_write_request;
    WriteResponse offset_write_response;
    offset_write_request.mutable_handle()->CopyFrom(create_response.handle());
    offset_write_request.set_offset(0);
    std::string offset_data = "Hi";
    offset_write_request.set_data(offset_data);
    
    brpc::Controller offset_write_cntl;
    stub.Write(&offset_write_cntl, &offset_write_request, &offset_write_response, NULL);  // Fixed this line
    
    ASSERT_FALSE(offset_write_cntl.Failed()) << "Offset write failed: " << offset_write_cntl.ErrorText();
    ASSERT_EQ(offset_write_response.status(), 0) << "Offset write should return success status";
    ASSERT_EQ(offset_write_response.bytes_written(), offset_data.size()) << "Should write all bytes";
    
    // Read the modified data
    ReadRequest modified_read_request;
    ReadResponse modified_read_response;
    modified_read_request.mutable_handle()->CopyFrom(create_response.handle());
    modified_read_request.set_offset(0);
    modified_read_request.set_size(13);
    
    brpc::Controller modified_read_cntl;
    stub.Read(&modified_read_cntl, &modified_read_request, &modified_read_response, NULL);
    
    ASSERT_FALSE(modified_read_cntl.Failed()) << "Modified read failed: " << modified_read_cntl.ErrorText();
    ASSERT_EQ(modified_read_response.status(), 0) << "Modified read should return success status";
    ASSERT_TRUE(modified_read_response.has_data()) << "Modified read response should contain data";
    ASSERT_EQ(modified_read_response.data(), "Hillo, ZTOFS!") << "Modified data should match";
}
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}