#include <gtest/gtest.h>
#include "server/meta/local_meta.h"
#include "server/storage/local_storage.h"
#include "server/common/errorcode.h"
#include "butil/file_util.h"
#include <memory>

using namespace ztofs;
using namespace ztofs::server;

class LocalStorageTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        mFsEnv.mountfd = open("/", O_DIRECTORY|O_RDONLY);
        mFsEnv.mountpath = "/";
        mLocalMeta = std::make_unique<LocalMeta>(&mFsEnv);
        mLocalStorage = std::make_unique<LocalStorage>(&mFsEnv);
        
        mkdir(mTestDir.c_str(), 0755);
    }

    void TearDown() override 
    {
        ASSERT_TRUE(butil::DeleteFile(butil::FilePath(mTestDir), true));
    }

public:
    FileHandle mRootHandle;
    FileSystemEnv mFsEnv;

    std::unique_ptr<LocalMeta> mLocalMeta;
    std::unique_ptr<LocalStorage> mLocalStorage;
    std::string mTestDir{"testdir"};
};


TEST_F(LocalStorageTest, CreateWriteAndRead) {
    std::string path = mTestDir+"/test_path";

    FileHandle parentHandle;
    int mount_id;
    EXPECT_GE(name_to_handle_at(AT_FDCWD, mTestDir.c_str(), parentHandle.RawHandle(), &mount_id, 0), 0);

    int file_fd = open_by_handle_at(mFsEnv.mountfd, parentHandle.RawHandle(), O_RDONLY | O_DIRECTORY);
    if (file_fd < 0) {
        LOG(ERROR) << "Failed to open parent handle: " << strerror(errno);
    }

    std::unique_ptr<FileHandle> handle= std::make_unique<FileHandle>();
    auto status = mLocalMeta->Create(parentHandle, "test_file", handle.get());
    ASSERT_EQ(status.error_code(), ZTO_OK);
    

    size_t writeCount = 0;
    status = mLocalStorage->Write(*handle, "test", 4, 0, &writeCount);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    ASSERT_EQ(4, writeCount);

    char buffer[10];
    size_t bytesRead = 0;
    status = mLocalStorage->Read(*handle, buffer, 4, 0, &bytesRead);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    ASSERT_EQ(4, bytesRead);
    ASSERT_EQ(strncmp(buffer, "test", 4), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}