#pragma once

#include "common_fs.h"
#include <json.hpp>
#include <regex>
#include <RemoteFile.h>
#include "Dir.h"
#include "OffsetCntName.h"
#include "OffsetCnt.h"
#include "File.h"
#include "VkException.h"
#include "FileName.h"
#include "FsException.h"
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <algorithm>

using json = nlohmann::json;

namespace vk_music_fs {
    namespace fs {
        template<typename TQueryMaker>
        class SearchDirMaker {
        public:
            SearchDirMaker(const std::shared_ptr<TQueryMaker> &queryMaker, const NumSearchFiles &numSearchFiles,
                           const Mp3Extension &ext): _queryMaker(queryMaker), _numSearchFiles(numSearchFiles), _ext(ext){
            }
            void createSearchDir(const DirPtr &parentDir, const std::string &dirName){
                createArtistOrSongSearchDir(parentDir, dirName, false);
            }

            void createArtistSearchDir(const DirPtr &parentDir, const std::string &dirName){
                createArtistOrSongSearchDir(parentDir, dirName, true);
            }

            struct OffsetsCnts{
                uint_fast32_t queryOffset;
                uint_fast32_t queryCnt;
                bool needClear;
            };

            OffsetsCnts processOffsetCnt(uint_fast32_t offset, uint_fast32_t cnt, uint_fast32_t newVal){
                if(newVal > cnt) {
                    return {offset + cnt, newVal - cnt, false};
                } else {
                    return {offset, newVal, true};
                }
            }

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
            };

            QueryParams parseQuery(const std::string &dirName){
                std::regex offsetRegex("^([0-9]{1,6})(?:-([0-9]{1,6}))?$");
                std::smatch mtc;
                if(std::regex_search(dirName, mtc, offsetRegex)){
                    if(mtc[2].matched) {
                        return QueryParams(
                                QueryParams::Type::TWO_NUMBERS, std::stoul(mtc[1].str()), std::stoul(mtc[2].str())
                        );
                    } else {
                        return QueryParams(QueryParams::Type::ONE_NUMBER, std::stoul(mtc[1].str()), 0);
                    }
                } else {
                    return QueryParams(QueryParams::Type::STRING, 0, 0);
                }
            }

            json makeSearchQuery(
                    const std::string &searchName,
                    uint_fast32_t offset, uint_fast32_t count
            ){
                std::string respStr;
                try{
                    respStr = _queryMaker->makeSearchQuery(searchName, offset, count);
                    return std::move(parseJson(respStr));
                } catch (const json::parse_error &err){
                    throw VkException("Error parsing JSON '" + respStr + "' when searching for " + searchName);
                }
            }

            json makeArtistSearchQuery(
                    const std::string &searchName,
                    uint_fast32_t offset, uint_fast32_t count
            ){
                std::string respStr;
                try{
                    respStr = _queryMaker->makeArtistSearchQuery(searchName, offset, count);
                    return std::move(parseJson(respStr));
                } catch (const json::parse_error &err){
                    throw VkException(
                            "Error parsing JSON '" + respStr + "' when searching in artist names for " + searchName
                    );
                }
            }

            json makeMyAudiosQuery(
                    uint_fast32_t offset, uint_fast32_t count
            ){
                std::string respStr;
                try{
                    respStr = _queryMaker->makeMyAudiosQuery(offset, count);
                    return std::move(parseJson(respStr));
                } catch (const json::parse_error &err){
                    throw VkException("Error parsing JSON '" + respStr + "' when obtaining my audios");
                }
            }

            void insertMp3sInDir(
                    const DirPtr &curDir, json returnedJson
            ) {
                auto resp = returnedJson["response"];
                for (const auto &item: resp["items"]) {
                    if(!static_cast<std::string>(item["url"]).empty()) {
                        FileName fname(item["artist"], item["title"], _ext);
                        while (curDir->hasItem(fname.getFilename())) {
                            fname.increaseNumberSuffix();
                        }
                        curDir->addItem(
                                std::make_shared<File>(
                                        fname.getFilename(),
                                        File::Type::MUSIC_FILE,
                                        RemoteFile{item["url"], item["owner_id"],
                                                   item["id"], fname.getArtist(), fname.getTitle()},
                                        curDir->getMaxFileNum(),
                                        curDir
                                )
                        );
                    }
                }
            }

            void createSearchDirInRoot(const DirPtr &parentDir, const std::string &dirName){
                auto data = makeSearchQuery(dirName, 0, _numSearchFiles);
                parentDir->addItem(
                        std::make_shared<Dir>(
                                dirName, Dir::Type::SEARCH_DIR,
                                OffsetCntName{0, _numSearchFiles, dirName}, DirWPtr{parentDir}
                        )
                );
                insertMp3sInDir(parentDir->getItem(dirName).dir(), data);
            }

            void createArtistSearchDirInRoot(const DirPtr &parentDir, const std::string &dirName){
                auto data = makeArtistSearchQuery(dirName, 0, _numSearchFiles);
                parentDir->addItem(
                        std::make_shared<Dir>(
                                dirName, Dir::Type::ARTIST_SEARCH_DIR,
                                OffsetCntName{0, _numSearchFiles, dirName}, DirWPtr{parentDir}
                        )
                );
                insertMp3sInDir(parentDir->getItem(dirName).dir(), data);
            }

            void createMyAudiosDir(const DirPtr &parentDir, const std::string &dirName){
                std::smatch mtc;
                uint_fast32_t offset;
                uint_fast32_t cnt;
                auto queryParams = parseQuery(dirName);
                if(queryParams.type != QueryParams::Type::STRING){
                    bool needClear = false;
                    if(queryParams.type == QueryParams::Type::TWO_NUMBERS) {
                        offset = queryParams.first;
                        cnt = queryParams.second;
                    } else {
                        auto offsetCnt = parentDir->getOffsetCnt();
                        auto res = processOffsetCnt(offsetCnt.getOffset(), offsetCnt.getCnt(), queryParams.first);
                        offset = res.queryOffset;
                        cnt = res.queryCnt;
                        needClear = res.needClear;
                    }

                    auto data = makeMyAudiosQuery(offset, cnt);

                    if(queryParams.type == QueryParams::Type::TWO_NUMBERS){
                        parentDir->clearContents();
                        parentDir->getOffsetCnt().setOffset(queryParams.first);
                        parentDir->getOffsetCnt().setCnt(queryParams.second);
                    } else {
                        if(needClear){
                            parentDir->clearContents();
                        }
                        parentDir->getOffsetCnt().setCnt(queryParams.first);
                    }

                    parentDir->removeCounter();
                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, Dir::Type::COUNTER_DIR, std::nullopt, DirWPtr{parentDir}
                            )
                    );
                    insertMp3sInDir(parentDir, data);
                } else {
                    throw FsException("Can't create dir in My Audios dir");
                }
            }

        private:
            void createArtistOrSongSearchDir(const DirPtr &parentDir, const std::string &dirName, bool byArtist){
                std::string searchName;
                uint_fast32_t offset;
                uint_fast32_t cnt;
                auto queryParams = parseQuery(dirName);
                if(queryParams.type != QueryParams::Type::STRING){
                    searchName = parentDir->getOffsetCntName().getName();
                    bool needClear = false;
                    if(queryParams.type == QueryParams::Type::TWO_NUMBERS) {
                        offset = queryParams.first;
                        cnt = queryParams.second;
                    } else {
                        auto offsetCntName = parentDir->getOffsetCntName();
                        auto res = processOffsetCnt(offsetCntName.getOffset(), offsetCntName.getCnt(), queryParams.first);
                        offset = res.queryOffset;
                        cnt = res.queryCnt;
                        needClear = res.needClear;
                    }

                    auto data =
                            (byArtist) ?
                            makeArtistSearchQuery(searchName, offset, cnt) :
                            makeSearchQuery(searchName, offset, cnt)
                    ;

                    if(queryParams.type == QueryParams::Type::TWO_NUMBERS){
                        parentDir->getOffsetCntName().setCnt(cnt);
                        parentDir->getOffsetCntName().setOffset(offset);
                        parentDir->clearContentsExceptNested();
                    } else {
                        if(needClear){
                            parentDir->clearContentsExceptNested();
                        }
                        parentDir->getOffsetCntName().setCnt(queryParams.first);
                    }

                    parentDir->removeCounter();
                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, Dir::Type::COUNTER_DIR,
                                    std::nullopt, DirWPtr{parentDir}
                            )
                    );
                    insertMp3sInDir(parentDir, data);
                } else {
                    offset = 0;
                    cnt = _numSearchFiles;
                    searchName = parentDir->getOffsetCntName().getName() + " " + dirName;

                    auto data =
                            (byArtist) ?
                            makeArtistSearchQuery(searchName, offset, cnt) :
                            makeSearchQuery(searchName, offset, cnt)
                    ;;

                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, (byArtist) ? Dir::Type::ARTIST_SEARCH_DIR : Dir::Type::SEARCH_DIR,
                                    OffsetCntName{offset, cnt, searchName}, DirWPtr{parentDir}
                            )
                    );
                    insertMp3sInDir(parentDir->getItem(dirName).dir(), std::move(data));
                }
            }

            json parseJson(const std::string &str){
                auto res = json::parse(str);
                if(res.find("error") != res.end()) {
                    throw VkException("VK returned error response '" + str + "'");
                }
                return std::move(res);
            }

            std::string _ext;
            std::shared_ptr<TQueryMaker> _queryMaker;
            uint_fast32_t _numSearchFiles;
        };
    }
}