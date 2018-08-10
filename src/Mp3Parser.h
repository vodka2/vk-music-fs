#pragma once

#include <string>
#include <memory>
#include <mpeg/mpegfile.h>
#include <mpeg/id3v2/id3v2framefactory.h>
#include <mpeg/id3v2/id3v2tag.h>
#include "common.h"

namespace vk_music_fs {
    template <typename TBuffer>
    class IOStream: public TagLib::IOStream{
    public:
        explicit IOStream(const std::shared_ptr<TBuffer> &buffer): _buffer(buffer), _getIndex(0){
        }

        TagLib::FileName name() const override{
            return "file.mp3";
        }

        TagLib::ByteVector readBlock(unsigned long length) override{
            auto tbuf = _buffer->read(_getIndex, length);
            _getIndex += length;
            return TagLib::ByteVector(reinterpret_cast<char*>(&tbuf[0]), tbuf.size());
        }

        void writeBlock(const TagLib::ByteVector &data) override{
        }

        void insert(const TagLib::ByteVector &data, unsigned long start, unsigned long replace) override{
            if(start == 0){
                ByteVect prep;
                prep.reserve(data.size());
                std::copy(data.begin(), data.end(), std::back_inserter(prep));
                _buffer->prepend(std::move(prep), replace);
            }
        }

        void removeBlock(unsigned long start, unsigned long length) override{
        }

        bool readOnly() const override{
            return false;
        }

        bool isOpen() const override{
            return true;
        }

        void seek(long offset, Position p) override{
            switch(p){
                case Beginning:
                    _getIndex = static_cast<uint_fast32_t>(offset);
                    break;
                case Current:
                    _getIndex = static_cast<uint_fast32_t>((int_fast32_t)_getIndex + offset);
                    break;
                case End:
                    break;
            }
        }

        long tell() const override{
            return _getIndex;
        }

        long length() override{
            return _buffer->getSize();
        }

        void truncate(long length) override{
        }
    private:
        std::shared_ptr<TBuffer> _buffer;
        uint_fast32_t _getIndex;
    };

    class Mp3Parser {
    public:
        Mp3Parser(Artist artist, Title title, TagSize tagSize);
        template <typename TBuffer>
        void parse(const std::shared_ptr<TBuffer> &buffer){
            auto strm = std::make_shared<IOStream<TBuffer>>(buffer);
            TagLib::MPEG::File f(strm.get(), TagLib::ID3v2::FrameFactory::instance());
            f.ID3v2Tag(true)->setExtraSize(_tagSize);
            if(f.ID3v2Tag()->artist().isEmpty()){
                f.ID3v2Tag()->setArtist(_artist.t);
            }
            if(f.ID3v2Tag()->title().isEmpty()) {
                f.ID3v2Tag()->setTitle(_title.t);
            }
            f.save();
        }
    private:
        Artist _artist;
        Title _title;
        TagSize _tagSize;
    };
}
