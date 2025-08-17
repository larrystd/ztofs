#include <gtest/gtest.h>
#include "server/meta/local_meta.h"
#include "server/common/errorcode.h"
#include "butil/file_util.h"

using namespace ztofs;
using namespace ztofs::server;

class LocalMetaTest : public ::testing::Test 
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
    std::string mTestDir{"testdir"};
};

TEST_F(LocalMetaTest, CreateAndRemove) {
    // 测试 LocalMeta::Create 函数是否能成功创建文件
    std::unique_ptr<FileHandle> file_handle(nullptr);
    std::string path = mTestDir+"/test_path";
    auto status = mLocalMeta.Create(path, file_handle.get());
    ASSERT_FALSE(status.ok());
    file_handle.reset(new FileHandle());
    status = mLocalMeta.Create(path, file_handle.get());
    ASSERT_TRUE(status.ok());
    status = mLocalMeta.Create(path, file_handle.get());
    ASSERT_EQ(status.error_code(), ZTO_FILE_ALREADY_EXISTS);

    status = mLocalMeta.Remove(*file_handle);
    ASSERT_TRUE(status.ok());
    status = mLocalMeta.Create(path, file_handle.get());
    ASSERT_TRUE(status.ok());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}