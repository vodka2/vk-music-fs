#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <FileProcessor.h>
#include <Mp3Parser.h>
#include "data/MusicData.h"
#include "data/FileM.h"
#include "data/StreamM.h"
#include "data/ThreadPoolM.h"
#include "data/Mp3Files.h"
#include "data/Writer.h"
#include "data/ParserM.h"
#include <diext/common_di.h>
#include <boost/di/extension/scopes/scoped.hpp>
#include <toolkit/tbytevectorstream.h>

namespace di = boost::di;

typedef vk_music_fs::Mp3Parser Mp3Parser;
using BlockingBuffer = vk_music_fs::BlockingBuffer<TestBlockCreator<10000>, FileM>;
typedef vk_music_fs::FileProcessor<StreamM, FileM, Mp3Parser, ThreadPoolM, BlockingBuffer, TestBlockCreator<5>> FileProcessor;

class FileProcessorMp3ParserT: public ::testing::Test {
public:
    vk_music_fs::Artist artist{"Justin Bieber"};
    vk_music_fs::Title title{"Baby"};
    vk_music_fs::TagSize prevTagSize{4096};
    auto_init(inj, (vk_music_fs::makeStorageInj(
        di::bind<vk_music_fs::Artist>.to(artist),
        di::bind<vk_music_fs::Title>.to(title),
        di::bind<vk_music_fs::TagSize>.to(prevTagSize)
    )));

    std::shared_ptr<FileProcessor> fp;
    std::shared_ptr<MusicData> data;
    std::shared_ptr<Writer> writer;
    std::future<void> finish;
    std::promise<void> finishPromise;

    void init(const ByteVect &dataVect){
        data = std::make_shared<MusicData>(dataVect, 10);
        writer = std::make_shared<Writer>();

        EXPECT_CALL(*inj.create<std::shared_ptr<StreamM>>(), read(testing::_)).WillRepeatedly(testing::Invoke(
                [&data = data] (Block blk) {
                    auto tmp = data->readData();
                    if (tmp) {
                        blk->arr() = *tmp;
                        blk->curSize() = tmp->size();
                    }
                }
        ));
        fp = inj.create<std::shared_ptr<FileProcessor>>();
        ON_CALL(*inj.create<std::shared_ptr<FileM>>(), write(testing::_)).WillByDefault(testing::Invoke([this] (auto data2){
            writer->write(data2->arr());
        }));
        ON_CALL(*inj.create<std::shared_ptr<FileM>>(), getSizeOnDisk()).WillByDefault(testing::Invoke([this] (){
            return writer->getSize();
        }));
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

    expectFinish();
    init(dataVect);
    fp->read(0, 1);
    waitForFinish();

    TagLib::ByteVector bvect(reinterpret_cast<char*>(&writer->data[0]), static_cast<unsigned int>(writer->data.size()));
    TagLib::ByteVectorStream strm(bvect);
    TagLib::MPEG::File f(&strm, TagLib::ID3v2::FrameFactory::instance());
    ASSERT_TRUE(f.hasID3v2Tag());
    EXPECT_EQ(f.ID3v2Tag(false)->title(), title.t);
    EXPECT_EQ(f.ID3v2Tag(false)->artist(), artist.t);
}