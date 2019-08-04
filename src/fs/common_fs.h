#pragma once

#include <common.h>
#include <unordered_map>
#include <variant>
#include <memory>

namespace vk_music_fs{
    class RemoteFile;
    namespace fs{
        class Dir;
        class File;
        class DirOrFile;
        class OffsetCnt;
        class OffsetCntName;
        class DummyDirMarker{};

        typedef std::optional<std::variant<OffsetCnt, OffsetCntName, DummyDirMarker>> DirExtra;
        typedef std::optional<std::variant<RemoteFile>> FileExtra;

        typedef std::shared_ptr<Dir> DirPtr;
        typedef std::weak_ptr<Dir> DirWPtr;
        typedef std::shared_ptr<File> FilePtr;

        struct QueryParams{
            enum class Type{
                TWO_NUMBERS,
                ONE_NUMBER,
                STRING
            } type;
            uint_fast32_t first;
            uint_fast32_t second;
            QueryParams(
                    Type type,
                    uint_fast32_t first,
                    uint_fast32_t second
            ): type(type), first(first), second(second){
            }
            bool operator ==(const QueryParams &other) const{
                return type == other.type && first == other.first && second == other.second;
            }
        };
    }
}
