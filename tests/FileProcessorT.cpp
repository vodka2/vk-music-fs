#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <FileProcessor.h>
#include "data/MusicData.h"
#include "data/FileM.h"
#include "data/StreamM.h"
#include "data/ThreadPoolM.h"
#include "data/Writer.h"
#include "data/ParserM.h"
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <net/HttpException.h>

namespace di = boost::di;

using vk_music_fs::ByteVect;

typedef vk_music_fs::FileProcessor<StreamM, FileM, ParserM, ThreadPoolM> FileProcessor;

class FileProcessorT: public ::testing::Test {
public:
    auto_init(inj, (di::make_injector<vk_music_fs::BoundPolicy>(
        di::bind<FileProcessor>.in(di::extension::scoped),
        di::bind<StreamM>.in(di::extension::scoped),
        di::bind<FileM>.in(di::extension::scoped),
        di::bind<ParserM>.in(di::extension::scoped),
        di::bind<ThreadPoolM>.in(di::extension::scoped)
    )));

    std::shared_ptr<FileProcessor> fp;
    std::shared_ptr<MusicData> data;
    std::shared_ptr<Writer> writer;
    std::future<void> finish;
    std::promise<void> finishPromise;

    void init(const ByteVect &dataVect){
        data = std::make_shared<MusicData>(dataVect, 1);
        writer = std::make_shared<Writer>();

        ON_CALL(*inj.create<std::shared_ptr<StreamM>>(), read()).WillByDefault(testing::Invoke([&data = data] {
            return data->readData();
        }));
        ON_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).WillByDefault(testing::Invoke([this] (auto data){
            writer->write(data);
        }));
        ON_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillByDefault(testing::Invoke([this] (){
            return writer->getSize();
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
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), setPrependSize(2));

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getTotalSize()).WillRepeatedly(testing::Return(dataVect.size()));

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(ByteVect{1, 2}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(dataVect));

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

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getTotalSize()).WillRepeatedly(testing::Return(dataVect.size()));

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

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getTotalSize()).WillRepeatedly(testing::Return(dataVect.size()));

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(ByteVect{}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(dataVect));

    expectFinish();
    init(dataVect);
    fp->read(0, 1);
    waitForFinish();
}

TEST_F(FileProcessorT, ReadBytesFromFile){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), read(1, 3)).WillRepeatedly(testing::Return(ByteVect{1, 2, 3}));
    expectFinish();
    init({});
    writer->write(ByteVect{0, 1, 2, 3});

    auto exp = ByteVect{1,2,3};
    EXPECT_EQ(fp->read(1, 3), exp);
    EXPECT_EQ(fp->read(1, 3), exp);

    waitForFinish();
}

TEST_F(FileProcessorT, ReadBytesFromStreamAndFile){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), read(1, 3)).WillOnce(testing::Return(ByteVect{1, 2, 3}));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(4, 1)).WillOnce(testing::Return(ByteVect{4}));

    expectFinish();
    init({});
    writer->write(ByteVect{0, 1, 2, 3});

    auto exp = ByteVect{1,2,3,4};
    EXPECT_EQ(fp->read(1, 4), exp);

    waitForFinish();
}

TEST_F(FileProcessorT, ReadBytesFromStream){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(1, 4)).WillOnce(testing::Return(ByteVect{1, 2, 3, 4}));

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

TEST_F(FileProcessorT, NonZeroInitialSize){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillRepeatedly(testing::Return(10));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getTotalSize()).WillRepeatedly(testing::Return(20));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getPrependSize()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), open(10, 20));
    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_)).Times(0);

    init({});
    fp->read(0, 1);

    expectFinish();
    waitForFinish();
}

TEST_F(FileProcessorT, NonZeroPrependSize){ //NOLINT
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillRepeatedly(testing::Return(10));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getTotalSize()).WillRepeatedly(testing::Return(20));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getPrependSize()).WillRepeatedly(testing::Return(5));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), open(5, 20));
    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_)).Times(0);

    init({});
    fp->read(0, 1);

    expectFinish();
    waitForFinish();
}

TEST_F(FileProcessorT, StreamOpenExc){ //NOLINT
    using vk_music_fs::net::HttpException;
    using vk_music_fs::RemoteException;
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), open(testing::_, testing::_)).WillOnce(testing::Invoke(
            [] (...) {
                throw HttpException("test exc");
            }
            ));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), close());
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), close());
    init({1,2,3,4,5});
    try {
        fp->read(0, 1);
        FAIL();
    } catch (const RemoteException &ex){
    }
}

TEST_F(FileProcessorT, StreamReadFirstExc){ //NOLINT
    using vk_music_fs::net::HttpException;
    using vk_music_fs::RemoteException;
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read()).WillOnce(testing::Invoke(
            [] (...) -> std::optional<ByteVect>{
                throw HttpException("test exc");
            }
    ));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), close());
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), close());
    init({1,2,3,4,5});
    try {
        fp->read(0, 1);
        FAIL();
    } catch (const RemoteException &ex){
    }
}

TEST_F(FileProcessorT, StreamReadMiddleExc){ //NOLINT
    using vk_music_fs::net::HttpException;
    using vk_music_fs::RemoteException;
    EXPECT_CALL(*inj.create<std::shared_ptr<ParserM>>(), parse(testing::_));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read()).WillRepeatedly(testing::Invoke(
            [] (...) -> std::optional<ByteVect>{
                return ByteVect{};
            }
    ));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(testing::_, testing::_)).WillOnce(testing::Invoke(
            [] (...) -> ByteVect{
                throw HttpException("test exc");
            }
    ));
    EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), close()).Times(testing::AtLeast(1));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), close());
    init({1,2,3,4,5});
    try {
        fp->read(0, 1);
        FAIL();
    } catch (const RemoteException &ex){
    }
}