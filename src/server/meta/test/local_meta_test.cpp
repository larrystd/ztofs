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
    EXPECT_GE(name_to_handle_at(AT_FDCWD, mTestDir.c_str(), parentHandle.RawHandle(), &mount_id, 0), 0);

    int file_fd = open_by_handle_at(mFsEnv.mountfd, parentHandle.RawHandle(), O_RDONLY | O_DIRECTORY);
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
    FileHandle temp;
    status = mLocalMeta->Lookup(parentHandle, "test_file", &temp);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    ASSERT_EQ(memcmp(temp.RawHandle()->f_handle, handle->RawHandle()->f_handle, handle->RawHandle()->handle_bytes), 0);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    status = mLocalMeta->Remove(parentHandle, "test_file");
    ASSERT_EQ(status.error_code(), ZTO_OK);
}

TEST_F(LocalMetaTest, SetAndGetAttr)
{
    // Create a test file first
    FileHandle parentHandle;
    int mount_id;
    EXPECT_GE(name_to_handle_at(AT_FDCWD, mTestDir.c_str(), parentHandle.RawHandle(), &mount_id, 0), 0);

    // Create a file to test SetAttr and GetAttr
    std::unique_ptr<FileHandle> fileHandle = std::make_unique<FileHandle>();
    auto status = mLocalMeta->Create(parentHandle, "setattr_test_file", fileHandle.get());
    ASSERT_EQ(status.error_code(), ZTO_OK);
    
    // Get initial attributes
    FileAttr initialAttr;
    status = mLocalMeta->GetAttr(*fileHandle, &initialAttr);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    
    // Prepare new attributes
    FileAttr newAttr = initialAttr;
    newAttr.mode = (newAttr.mode & ~0777) | 0644; // Set specific permissions
    newAttr.atime = initialAttr.atime + 100;      // Change access time
    newAttr.mtime = initialAttr.mtime + 200;      // Change modification time
    
    // Set the new attributes
    status = mLocalMeta->SetAttr(*fileHandle, newAttr);
    EXPECT_EQ(status.error_code(), ZTO_OK);
    
    // Get attributes after setting
    FileAttr retrievedAttr;
    status = mLocalMeta->GetAttr(*fileHandle, &retrievedAttr);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    
    // Verify that retrieved attributes match what we set
    EXPECT_EQ(retrievedAttr.mode & 0777, 0644);
    EXPECT_EQ(retrievedAttr.atime, newAttr.atime);
    EXPECT_EQ(retrievedAttr.mtime, newAttr.mtime);
    EXPECT_EQ(retrievedAttr.uid, newAttr.uid);
    EXPECT_EQ(retrievedAttr.gid, newAttr.gid);
    
    // Test error conditions
    // Test SetAttr with null attribute (compile error, so not applicable)
    // Test GetAttr with null attribute pointer
    status = mLocalMeta->GetAttr(*fileHandle, nullptr);
    EXPECT_EQ(status.error_code(), EINVAL);
    
    // Test with invalid handles
    FileAttr dummyAttr;
    status = mLocalMeta->SetAttr(FileHandle(), dummyAttr);
    EXPECT_NE(status.error_code(), ZTO_OK);
    
    status = mLocalMeta->GetAttr(FileHandle(), &dummyAttr);
    EXPECT_NE(status.error_code(), ZTO_OK);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}