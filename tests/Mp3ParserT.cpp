#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/asio.hpp>
#include <boost/di.hpp>
#include <Mp3Parser.h>
#include <toolkit/tbytevectorstream.h>
#include "data/Mp3Files.h"
#include <boost/di/extension/scopes/scoped.hpp>

namespace di = boost::di;
using vk_music_fs::ByteVect;

class BlockingBufferM{
public:
    BlockingBufferM(){} //NOLINT
    MOCK_CONST_METHOD2(read, ByteVect (uint_fast32_t offset, uint_fast32_t len));
    MOCK_CONST_METHOD2(prepend, void (ByteVect vect, uint_fast32_t replace)); //NOLINT
    MOCK_CONST_METHOD0(getSize, uint_fast32_t());
};

typedef vk_music_fs::Mp3Parser Mp3Parser;

class Mp3Buffer{
public:
    Mp3Buffer(uint8_t *arr, uint_fast32_t len): _arr(arr), _len(len){}
    ByteVect read(uint_fast32_t offset, uint_fast32_t len){
        ByteVect res;
        std::copy(_arr + offset, _arr + offset + len, std::back_inserter(res));
        return res;
    }
    uint_fast32_t getSize(){
        return _len;
    }
private:
    uint8_t *_arr;
    uint_fast32_t _len;
};

class Mp3ParserT: public ::testing::Test {
public:
    vk_music_fs::Artist artist{"Justin Bieber"};
    vk_music_fs::Title title{"Baby"};
    vk_music_fs::TagSize prevTagSize{4096};
    di::injector<
       std::shared_ptr<Mp3Parser>,
       std::shared_ptr<BlockingBufferM>
    > inj = di::make_injector(
            di::bind<Mp3Parser>.in(di::extension::scoped),
            di::bind<BlockingBufferM>.in(di::extension::scoped),
            di::bind<vk_music_fs::Artist>.to(artist),
            di::bind<vk_music_fs::Title>.to(title),
            di::bind<vk_music_fs::TagSize>.to(prevTagSize)
    );

    void init(const std::shared_ptr<Mp3Buffer> &buf){
        EXPECT_CALL(*inj.create<std::shared_ptr<BlockingBufferM>>(), read(testing::_, testing::_)).WillRepeatedly(
                testing::Invoke([&buf] (uint_fast32_t offset, uint_fast32_t len) {
                    return buf->read(offset, len);
                })
        );
        EXPECT_CALL(*inj.create<std::shared_ptr<BlockingBufferM>>(), getSize()).WillRepeatedly(
                testing::Invoke([&buf] {
                    return buf->getSize();
                })
        );
    }
};

TEST_F(Mp3ParserT, AddID3){ //NOLINT
    auto buf = std::make_shared<Mp3Buffer>(fileMp3NoID3, fileMp3NoID3Len);
    init(buf);
    ByteVect prepVect;
    uint_fast32_t replaceLen;
    EXPECT_CALL(*inj.create<std::shared_ptr<BlockingBufferM>>(), prepend(testing::_, testing::_)).WillOnce(
            testing::Invoke([&prepVect, &replaceLen] (auto vect, uint_fast32_t replace) {
                prepVect = vect;
                replaceLen = replace;
            })
    );
    inj.create<std::shared_ptr<Mp3Parser>>()->parse<BlockingBufferM>(inj.create<std::shared_ptr<BlockingBufferM>>());
    std::vector<uint8_t> vect;
    std::copy(prepVect.cbegin(), prepVect.cend(), std::back_inserter(vect));
    std::copy(fileMp3NoID3 + replaceLen, fileMp3NoID3 + fileMp3NoID3Len, std::back_inserter(vect));
    TagLib::ByteVector bvect(reinterpret_cast<char*>(&vect[0]), static_cast<unsigned int>(vect.size()));
    TagLib::ByteVectorStream strm(bvect);
    TagLib::MPEG::File f(&strm, TagLib::ID3v2::FrameFactory::instance());
    ASSERT_TRUE(f.hasID3v2Tag());
    EXPECT_EQ(f.ID3v2Tag()->title(), title.t);
    EXPECT_EQ(f.ID3v2Tag()->artist(), artist.t);
}

TEST_F(Mp3ParserT, DontAddID3){ //NOLINT
    auto buf = std::make_shared<Mp3Buffer>(fileMp3WithID3, fileMp3WithID3Len);
    init(buf);
    ByteVect prepVect;
    uint_fast32_t replaceLen;
    EXPECT_CALL(*inj.create<std::shared_ptr<BlockingBufferM>>(), prepend(testing::_, testing::_)).WillOnce(
            testing::Invoke([&prepVect, &replaceLen] (auto vect, uint_fast32_t replace) {
                prepVect = vect;
                replaceLen = replace;
            })
    );
    inj.create<std::shared_ptr<Mp3Parser>>()->parse<BlockingBufferM>(inj.create<std::shared_ptr<BlockingBufferM>>());
    std::vector<uint8_t> vect;
    std::copy(fileMp3WithID3, fileMp3WithID3 + fileMp3WithID3Len, std::back_inserter(vect));
    TagLib::ByteVector bvect(reinterpret_cast<char*>(&vect[0]), static_cast<unsigned int>(vect.size()));
    TagLib::ByteVectorStream strm1(bvect);
    TagLib::MPEG::File f1(&strm1, TagLib::ID3v2::FrameFactory::instance());
    auto prevSize = f1.ID3v2Tag()->render(4).size();
    vect.clear();

    std::copy(prepVect.cbegin(), prepVect.cend(), std::back_inserter(vect));
    std::copy(fileMp3WithID3 + replaceLen, fileMp3WithID3 + fileMp3WithID3Len, std::back_inserter(vect));
    bvect = TagLib::ByteVector(reinterpret_cast<char*>(&vect[0]), static_cast<unsigned int>(vect.size()));
    TagLib::ByteVectorStream strm2(bvect);
    TagLib::MPEG::File f2(&strm2, TagLib::ID3v2::FrameFactory::instance());
    EXPECT_EQ(f2.ID3v2Tag()->title(), std::string("World"));
    EXPECT_EQ(f2.ID3v2Tag()->artist(), std::string("Hello"));
    EXPECT_EQ(f2.ID3v2Tag()->render(4).size(), prevSize + prevTagSize.t);
}