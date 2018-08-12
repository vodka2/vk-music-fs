#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <FileProcessor.h>
#include "data/MusicData.h"
#include "data/FileM.h"
#include "data/StreamM.h"
#include "data/ThreadPoolM.h"
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>

namespace di = boost::di;

using vk_music_fs::ByteVect;

class ParserM{
public:
    ParserM(){} //NOLINT
    MOCK_CONST_METHOD1(parse, void(const std::shared_ptr<vk_music_fs::BlockingBuffer> &vect));
};

typedef vk_music_fs::FileProcessor<StreamM, FileM, ParserM, ThreadPoolM> FileProcessor;

class FileProcessorT: public ::testing::Test {
public:
    di::injector<
        std::shared_ptr<FileProcessor>,
        std::shared_ptr<StreamM>,
        std::shared_ptr<FileM>,
        std::shared_ptr<ParserM>,
        std::shared_ptr<ThreadPoolM>
    > inj = di::make_injector(
        di::bind<FileProcessor>.in(di::extension::scoped),
        di::bind<StreamM>.in(di::extension::scoped),
        di::bind<FileM>.in(di::extension::scoped),
        di::bind<ParserM>.in(di::extension::scoped),
        di::bind<ThreadPoolM>.in(di::extension::scoped)
    );

    std::shared_ptr<FileProcessor> fp;
    std::shared_ptr<MusicData> data;
    std::future<void> finish;
    std::promise<void> finishPromise;

    void init(const ByteVect &dataVect){
        data = std::make_shared<MusicData>(dataVect, 1);

        EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), getSize()).WillOnce(testing::Return(dataVect.size()));
        EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read()).WillRepeatedly(testing::Invoke([&data = data] {
            return data->readData();
        }));
        fp = inj.create<std::shared_ptr<FileProcessor>>();
    }

    void expectFinish(){
        finish = finishPromise.get_future();
        EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), finish()).WillOnce(testing::Invoke([this] {
            finishPromise.set_value();
        }));
    }

    void waitForFinish(){
        finish.wait();
        inj.create<std::shared_ptr<ThreadPoolM>>()->tp.join();
    }
};

TEST_F(FileProcessorT, Prepend){ //NOLINT
    ByteVect dataVect{1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3};

    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_)).WillRepeatedly(testing::Invoke([&dataVect] (
        const std::shared_ptr<vk_music_fs::BlockingBuffer> &buf
    ) {
        buf->read(3, 5);
        buf->prepend(buf->read(10, 2), 0);
        buf->read(0, dataVect.size());
    }));

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(ByteVect{1, 2}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(dataVect));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillOnce(testing::Return(dataVect.size()));

    expectFinish();
    init(dataVect);
    fp->read(0, 1);
    waitForFinish();
}

TEST_F(FileProcessorT, NoPrepend){ //NOLINT
    ByteVect dataVect{1, 2, 3, 4, 5};

    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_)).WillRepeatedly(testing::Invoke([&dataVect] (
            const std::shared_ptr<vk_music_fs::BlockingBuffer> &buf
    ) {
        buf->read(3, 1);
        buf->read(0, dataVect.size());
    }));

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(ByteVect{}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(dataVect));

    expectFinish();
    init(dataVect);
    fp->read(0, 1);
    waitForFinish();
}

TEST_F(FileProcessorT, OnEOF){ //NOLINT
    ByteVect dataVect{1, 2, 3, 4, 5};

    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_)).WillRepeatedly(testing::Invoke([&dataVect] (
            const std::shared_ptr<vk_music_fs::BlockingBuffer> &buf
    ) {
        buf->read(3, 1);
        EXPECT_EQ(buf->read(4, 10).size(), 1);
        EXPECT_EQ(buf->read(4, 10).size(), 1);
    }));

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(ByteVect{}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(dataVect));

    expectFinish();
    init(dataVect);
    fp->read(0, 1);
    waitForFinish();
}

TEST_F(FileProcessorT, ReadBytesFromFile){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), read(1, 3)).WillOnce(testing::Return(ByteVect{1, 2, 3}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillOnce(testing::Return(4));

    expectFinish();
    init({});

    auto exp = ByteVect{1,2,3};
    EXPECT_EQ(fp->read(1, 3), exp);

    waitForFinish();
}

TEST_F(FileProcessorT, ReadBytesFromStreamAndFile){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), read(1, 3)).WillOnce(testing::Return(ByteVect{1, 2, 3}));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(4, 1)).WillOnce(testing::Return(ByteVect{4}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillOnce(testing::Return(4));

    expectFinish();
    init({});

    auto exp = ByteVect{1,2,3,4};
    EXPECT_EQ(fp->read(1, 4), exp);

    waitForFinish();
}

TEST_F(FileProcessorT, ReadBytesFromStream){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(1, 4)).WillOnce(testing::Return(ByteVect{1, 2, 3, 4}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillOnce(testing::Return(1));

    expectFinish();
    init({});

    auto exp = ByteVect{1,2,3,4};
    EXPECT_EQ(fp->read(1, 4), exp);

    waitForFinish();
}

TEST_F(FileProcessorT, Close){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).Times(testing::AtLeast(1));
    init({});
    fp->read(0, 1);

    finish = finishPromise.get_future();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), close()).WillOnce(testing::Invoke([this] {
        finishPromise.set_value();
    }));

    fp->close();

    waitForFinish();
}