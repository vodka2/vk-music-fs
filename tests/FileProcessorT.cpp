#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <FileProcessor.h>
#include <boost/asio.hpp>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>

namespace di = boost::di;

using vk_music_fs::ByteVect;

class ThreadPoolM{
public:
    boost::asio::thread_pool tp;
    std::mutex mutex;
    ThreadPoolM(){}; //NOLINT
    template <typename T>
    void post(T func){ //NOLINT
        std::scoped_lock<std::mutex> lock(mutex);
        boost::asio::post(tp, func);
    }
};

class FileM{
public:
    FileM(){} //NOLINT
    MOCK_CONST_METHOD1(write, void(ByteVect vect)); //NOLINT
    MOCK_CONST_METHOD0(finish, void());
    MOCK_CONST_METHOD0(close, void());
};

class ParserM{
public:
    ParserM(){} //NOLINT
    MOCK_CONST_METHOD1(parse, void(const std::shared_ptr<vk_music_fs::BlockingBuffer> &vect));
};

class StreamM{
public:
    StreamM(){} //NOLINT
    MOCK_CONST_METHOD0(read, std::optional<ByteVect>());
    MOCK_CONST_METHOD0(getSize, uint_fast32_t());
};


typedef vk_music_fs::FileProcessor<StreamM, FileM, ParserM, ThreadPoolM> FileProcessor;

class MusicData{
private:
    ByteVect _data;
    uint_fast32_t _offset;
    const int LEN = 2;
public:
    explicit MusicData(ByteVect data) : _data(std::move(data)), _offset(0){}
    std::optional<ByteVect> readData(){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ByteVect res;
        if(_offset >= _data.size()){
            return std::nullopt;
        }
        uint_fast32_t realLen = std::min(_offset + LEN, _data.size());
        std::copy(_data.cbegin() + _offset, _data.cbegin() + realLen, std::back_inserter(res));
        _offset += LEN;
        return res;
    }
};

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
        di::bind<ThreadPoolM>.in(di::extension::scoped)
    );

    std::shared_ptr<FileProcessor> fp;
    std::shared_ptr<MusicData> data;
    std::future<void> finish;
    std::promise<void> finishPromise;

    void init(const ByteVect &dataVect){
        data = std::make_shared<MusicData>(dataVect);

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

    void end(){
        fp->openFile().wait();
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
        buf->prepend(buf->read(10, 2));
        buf->read(0, dataVect.size());
    }));

    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(ByteVect{1, 2}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(dataVect));

    expectFinish();
    init(dataVect);
    end();
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
    end();
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
    end();
    waitForFinish();
}