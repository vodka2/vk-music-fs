#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/asio.hpp>
#include <boost/di.hpp>
#include <diext/common_di.h>
#include <common/common.h>
#include <boost/di/extension/scopes/scoped.hpp>
#include <BlockingBuffer.h>
#include "data/ThreadPoolM.h"
#include "data/FileM.h"

namespace di = boost::di;
using vk_music_fs::ByteVect;

using BlockingBuffer = vk_music_fs::BlockingBuffer<TestBlockCreator<2>, FileM>;
typedef TestBlockCreator<2> BlockCreator;

class BlockingBufferT: public ::testing::Test {
public:
    auto_init(inj, (vk_music_fs::makeStorageInj()));

    std::shared_ptr<FileM> file;
    std::shared_ptr<BlockingBuffer> buffer;
    std::shared_ptr<ThreadPoolM> pool;
    Block block;
    std::future<void> finish;
    std::promise<void> finishPromise;

    BlockingBufferT() :
        file(inj.create<std::shared_ptr<FileM>>()),
        buffer(inj.create<std::shared_ptr<BlockingBuffer>>()),
        block(inj.create<std::shared_ptr<BlockCreator>>()->create()),
        pool(inj.create<std::shared_ptr<ThreadPoolM>>())
    {
        finish = finishPromise.get_future();
    }

    void setFinish(){
        finishPromise.set_value();
    }

    void waitForFinish(){
        finish.wait();
        inj.create<std::shared_ptr<ThreadPoolM>>()->tp.join();
    }

    void prepare6ElementBuffer(){
        EXPECT_CALL(*file, write(testing::Eq(ByteVect{3, 4})));
        EXPECT_CALL(*file, write(testing::Eq(ByteVect{5, 6})));
        pool->post([this] {
            buffer->setSize(6);
            block->arr() = {1, 2};
            block->curSize() = 2;
            buffer->append(block);
            block->arr() = {3, 4};
            block->curSize() = 2;
            buffer->append(block);
            block->arr() = {5, 6};
            block->curSize() = 2;
            buffer->append(block);
            buffer->setEOF();
            setFinish();
        });
    }
};

TEST_F(BlockingBufferT, readFromFileOnce) {
    EXPECT_CALL(*file, read(1, testing::Matcher<Block>(testing::_))).WillOnce([] (uint_fast32_t, Block blk) {
        blk->arr() = ByteVect{4, 5, 6};
        blk->curSize() = 3;
    });

    prepare6ElementBuffer();
    waitForFinish();

    EXPECT_THAT(buffer->read(3, 3), testing::ElementsAre(4, 5, 6));
}

TEST_F(BlockingBufferT, readFromFileTwice) {
    EXPECT_CALL(*file, read(0, testing::Matcher<Block>(testing::_))).WillOnce([] (uint_fast32_t, Block blk) {
        blk->arr() = ByteVect{3};
        blk->curSize() = 1;
    });
    EXPECT_CALL(*file, read(1, testing::Matcher<Block>(testing::_))).WillOnce([] (uint_fast32_t, Block blk) {
        blk->arr() = ByteVect{3, 4};
        blk->curSize() = 2;
    });

    prepare6ElementBuffer();
    waitForFinish();

    EXPECT_THAT(buffer->read(2, 1), testing::ElementsAre(3));
    EXPECT_THAT(buffer->read(3, 2), testing::ElementsAre(3, 4));
}


TEST_F(BlockingBufferT, readFully) {
    EXPECT_CALL(*file, read(0, testing::Matcher<Block>(testing::_))).WillOnce([] (uint_fast32_t, Block blk) {
        ByteVect t{3, 4, 5, 6};
        std::copy(t.cbegin(), t.cend(), blk->addrCurSize());
        blk->curSize() += 4;
    });

    prepare6ElementBuffer();
    waitForFinish();

    EXPECT_THAT(buffer->read(0, 6), testing::ElementsAre(1, 2, 3, 4, 5, 6));
}
