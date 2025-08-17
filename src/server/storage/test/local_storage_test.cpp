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
        // Create test directory
        mkdir(mTestDir.c_str(), 0755);
    }

    void TearDown() override 
    {
        ASSERT_TRUE(butil::DeleteFile(butil::FilePath(mTestDir), true));
    }

    LocalMeta mLocalMeta;
    LocalStorage mLocalStorage;
    std::string mTestDir{"testdir"};
};

TEST_F(LocalStorageTest, CreateWriteAndRead) {
    std::string path = mTestDir+"/test_path";

    std::unique_ptr<FileHandle> file_handle = std::make_unique<FileHandle>();
    butil::Status status = mLocalMeta.Create(path, file_handle.get());
    ASSERT_TRUE(status.ok());
    

    status = mLocalStorage.Write(file_handle.get(), "test", 4);
    ASSERT_EQ(status.error_code(), ZTO_OK);

    char buffer[10];
    size_t bytes_read = 0;
    status = mLocalStorage.Read(file_handle.get(), buffer, 4, &bytes_read);
    ASSERT_EQ(status.error_code(), ZTO_OK);
    ASSERT_TRUE(strcmp(buffer, "test"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}