#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <FileProcessor.h>
#include <Mp3Parser.h>
#include "data/MusicData.h"
#include "data/FileM.h"
#include "data/StreamM.h"
#include "data/ThreadPoolM.h"
#include "data/Mp3Files.h"
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <toolkit/tbytevectorstream.h>

namespace di = boost::di;

class ParserM{
public:
    ParserM(){} //NOLINT
    MOCK_CONST_METHOD1(parse, void(const std::shared_ptr<vk_music_fs::BlockingBuffer> &vect));
};

typedef vk_music_fs::Mp3Parser Mp3Parser;
typedef vk_music_fs::FileProcessor<StreamM, FileM, Mp3Parser, ThreadPoolM> FileProcessor;

class FileProcessorMp3ParserT: public ::testing::Test {
public:
    vk_music_fs::Artist artist{"Justin Bieber"};
    vk_music_fs::Title title{"Baby"};
    vk_music_fs::TagSize prevTagSize{4096};
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
        di::bind<ThreadPoolM>.in(di::extension::scoped),
        di::bind<Mp3Parser>.in(di::extension::scoped),
        di::bind<vk_music_fs::Artist>.to(artist),
        di::bind<vk_music_fs::Title>.to(title),
        di::bind<vk_music_fs::TagSize>.to(prevTagSize)
    );

    std::shared_ptr<FileProcessor> fp;
    std::shared_ptr<MusicData> data;
    std::future<void> finish;
    std::promise<void> finishPromise;

    void init(const ByteVect &dataVect){
        data = std::make_shared<MusicData>(dataVect, 10);

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

TEST_F(FileProcessorMp3ParserT, AddID3){ //NOLINT
    ByteVect dataVect(fileMp3NoID3, fileMp3NoID3 + fileMp3NoID3Len);

    ByteVect resVect;
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).WillRepeatedly(testing::Invoke([&resVect] (ByteVect vect) {
        std::copy(vect.cbegin(), vect.cend(), std::back_inserter(resVect));
    }));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileM>>(), getSize()).WillOnce(testing::Return(1));

    expectFinish();
    init(dataVect);
    fp->read(0, 1);
    waitForFinish();

    TagLib::ByteVector bvect(reinterpret_cast<char*>(&resVect[0]), static_cast<unsigned int>(resVect.size()));
    TagLib::ByteVectorStream strm(bvect);
    TagLib::MPEG::File f(&strm, TagLib::ID3v2::FrameFactory::instance());
    ASSERT_TRUE(f.hasID3v2Tag());
    EXPECT_EQ(f.ID3v2Tag(false)->title(), title.t);
    EXPECT_EQ(f.ID3v2Tag(false)->artist(), artist.t);
}