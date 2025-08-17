#include <gtest/gtest.h>
#include "server/meta/local_meta.h"

using namespace ztofs::server;

class LocalMetaTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up code here
    }

    void TearDown() override {
        // Clean up code here
    }

    LocalMeta local_meta;
};

TEST_F(LocalMetaTest, CreateSuccessfully) {
    // 测试 LocalMeta::Create 函数是否能成功创建对象
    std::unique_ptr<FileHandle> file_handle(nullptr);
    auto status = local_meta.Create("test_path", file_handle.get());
    ASSERT_FALSE(status.ok());
    file_handle.reset(new FileHandle());
    status = local_meta.Create("test_path", file_handle.get());
    ASSERT_TRUE(status.ok());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}