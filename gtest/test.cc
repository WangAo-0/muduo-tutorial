#include "gtest/gtest.h"
#include "../IO/File.h"
#include "../IO/IOEventLoop.h"
#include <memory>
#include <string>

class FileTest : public testing::Test {
protected:
    void SetUp() override {
        loop = std::make_shared<IOEventLoop>();
        file =std::make_shared<File>(loop.get(), "./test.txt");
    }

    void TearDown() override {
    }

    std::shared_ptr<IOEventLoop> loop;
    std::shared_ptr<File> file;
};

TEST_F(FileTest, WriteAndReadFile) {
    const std::string content = "hello world";
    file->registerFile();
    file->writeFile(content);
    file->readFile(content.size(), 0); 
    loop->loop();
    ASSERT_EQ(content, file->readBuffer_.data()) << "字符串必须相等";
    std::cout << "文件写/读成功" << std::endl;
}

