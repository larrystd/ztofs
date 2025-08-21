#include <fcntl.h>
#include <gtest/gtest.h>
#include <memory>
#include "server/meta/local_meta.h"
#include "server/common/errorcode.h"
#include "butil/file_util.h"
#include <stdio.h>

using namespace ztofs;
using namespace ztofs::server;

class LocalMetaTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        mFsEnv.mountfd = open("/", O_DIRECTORY|O_RDONLY);
        mFsEnv.mountpath = "/";
        mLocalMeta = std::make_unique<LocalMeta>(&mFsEnv);
        
        mkdir(mTestDir.c_str(), 0755);
    }

    void TearDown() override 
    {
        ASSERT_TRUE(butil::DeleteFile(butil::FilePath(mTestDir), true));
    }
    FileHandle mRootHandle;
    FileSystemEnv mFsEnv;

    std::unique_ptr<LocalMeta> mLocalMeta;
    std::string mTestDir{"/testdir"};
};
TEST_F(LocalMetaTest, CreateAndRemove) 
{
    FileHandle parentHandle;
    int mount_id;
    EXPECT_GE(name_to_handle_at(AT_FDCWD, mTestDir.c_str(), parentHandle.rawhandle.get(), &mount_id, 0), 0);

    int file_fd = open_by_handle_at(mFsEnv.mountfd, parentHandle.rawhandle.get(), O_RDONLY | O_DIRECTORY);
    if (file_fd < 0) {
        LOG(ERROR) << "Failed to open parent handle: " << strerror(errno);
    }

    std::unique_ptr<FileHandle> handle= std::make_unique<FileHandle>();
    auto status = mLocalMeta->Create(parentHandle, "test_file", handle.get());
    ASSERT_EQ(status.error_code(), ZTO_OK);
    status = mLocalMeta->Create(parentHandle, "test_file", handle.get());
    ASSERT_EQ(status.error_code(), ZTO_CREATE_FAILED);

    status = mLocalMeta->Remove(parentHandle, "test_file");
    ASSERT_EQ(status.error_code(), ZTO_OK);
    status = mLocalMeta->Create(parentHandle, "test_file", handle.get());
    ASSERT_EQ(status.error_code(), ZTO_OK);
    status = mLocalMeta->Open(*handle, O_RDONLY);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    status = mLocalMeta->Remove(parentHandle, "test_file");
    ASSERT_EQ(status.error_code(), ZTO_OK);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}