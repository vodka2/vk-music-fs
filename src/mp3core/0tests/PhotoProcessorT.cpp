#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <PhotoProcessor.h>
#include "data/MusicData.h"
#include "data/FileM.h"
#include "data/StreamM.h"
#include "data/ThreadPoolM.h"
#include "data/Writer.h"
#include "data/ParserM.h"
#include "mp3core/0tests/data/TestBlockCreator.h"
#include <diext/common_di.h>
#include <boost/di/extension/scopes/scoped.hpp>
#include <BlockingBuffer.h>

namespace di = boost::di;

using vk_music_fs::ByteVect;

using TestBlockCreatorProc = TestBlockCreator<5000>;

typedef vk_music_fs::PhotoProcessor<StreamM, FileM, TestBlockCreatorProc> PhotoProcessor;

class PhotoProcessorT: public ::testing::Test {
public:
    auto_init(inj, (vk_music_fs::makeStorageInj()));

    std::shared_ptr<PhotoProcessor> proc;
    std::shared_ptr<MusicData> data;
    std::shared_ptr<Writer> writer;
    std::shared_ptr<TestBlockCreatorProc> creatorProc;

    void init(const ByteVect &dataVect){
        creatorProc = inj.create<std::shared_ptr<TestBlockCreatorProc>>();
        creatorProc->setSize(dataVect.size());
        data = std::make_shared<MusicData>(dataVect, 1);
        writer = std::make_shared<Writer>();

        ON_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(testing::_)).WillByDefault(testing::Invoke([&data = data] (auto buf) {
            auto t = data->readData();
            if (t) {
                buf->arr() = *t;
                buf->curSize() += t->size();
            }
        }));
        ON_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).WillByDefault(testing::Invoke([this] (auto buf){
            writer->write(buf->arr());
        }));
        ON_CALL(*inj.create<std::shared_ptr<FileM>>(), getSizeOnDisk()).WillByDefault(testing::Invoke([this] (){
            return writer->getSize();
        }));
        proc = inj.create<std::shared_ptr<PhotoProcessor>>();
    }
};

TEST_F(PhotoProcessorT, ReadBytesFromFile){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), read(1, 3)).WillRepeatedly(testing::Return(ByteVect{1, 2, 3}));
    init({});
    writer->write(ByteVect{0, 1, 2, 3});
    ON_CALL(*inj.create<std::shared_ptr<FileM>>(), getUriSize()).WillByDefault(testing::Invoke([this] (){
        return writer->getSize();
    }));

    auto exp = ByteVect{1,2,3};
    EXPECT_EQ(proc->read(1, 3), exp);
    EXPECT_EQ(proc->read(1, 3), exp);
}

TEST_F(PhotoProcessorT, ReadBytesFromStreamAndFile){ //NOLINT
    auto data = ByteVect{0, 1, 2, 3};
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), read(1, 3)).WillRepeatedly(testing::Return(ByteVect{1, 2, 3}));
    init(data);
    ON_CALL(*inj.create<std::shared_ptr<FileM>>(), getUriSize()).WillByDefault(testing::Invoke([this, &data] (){
        return data.size();
    }));

    auto exp = ByteVect{1,2,3};
    EXPECT_EQ(proc->read(1, 3), exp);
    EXPECT_EQ(proc->read(1, 3), exp);
}

TEST_F(PhotoProcessorT, ReadFromStreamThrowRemoteEx){ //NOLINT
    init({});
    ON_CALL(*inj.create<std::shared_ptr<FileM>>(), getUriSize()).WillByDefault(testing::Invoke([this] (){
        return 100;
    }));
    ON_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(testing::_)).WillByDefault(testing::Invoke([] (...) {
        throw vk_music_fs::MusicFsException("");
    }));

    try {
        proc->read(1, 3);
        FAIL();
    } catch (const vk_music_fs::RemoteException &ex) {}
}