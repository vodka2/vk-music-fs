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
            bool createSearchDir(const DirPtr &parentDir, const std::string &dirName){
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

                    auto data = makeSearchQuery(searchName, offset, cnt);

                    if(queryParams.type == QueryParams::Type::TWO_NUMBERS){
                        parentDir->getOffsetCntName().setCnt(cnt);
                        parentDir->getOffsetCntName().setOffset(offset);
                        parentDir->clearContents();
                    } else {
                        if(needClear){
                            parentDir->clearContents();
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

                    auto data = makeSearchQuery(searchName, offset, cnt);

                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, Dir::Type::SEARCH_DIR,
                                    OffsetCntName{offset, cnt, searchName}, DirWPtr{parentDir}
                            )
                    );
                    insertMp3sInDir(parentDir->getItem(dirName).dir(), std::move(data));
                }
                return true;
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
                    auto initialFileName = genFileName(item["artist"], item["title"]);
                    auto fileName = initialFileName + _ext;
                    uint_fast32_t i = 2;
                    while (curDir->hasItem("fileName")) {
                        fileName = initialFileName + "_" + std::to_string(i) + _ext;
                        i++;
                    }
                    curDir->addItem(
                            std::make_shared<File>(
                                    fileName,
                                    File::Type::MUSIC_FILE,
                                    RemoteFile{item["url"], item["owner_id"],
                                               item["id"], item["artist"], item["title"]},
                                    curDir->getMaxFileNum(),
                                    curDir
                            )
                    );
                }
            }

            bool createSearchDirInRoot(const DirPtr &parentDir, const std::string &dirName){
                auto data = makeSearchQuery(dirName, 0, _numSearchFiles);
                parentDir->addItem(
                        std::make_shared<Dir>(
                                dirName, Dir::Type::SEARCH_DIR,
                                OffsetCntName{0, _numSearchFiles, dirName}, DirWPtr{parentDir}
                        )
                );
                insertMp3sInDir(parentDir->getItem(dirName).dir(), data);
                return true;
            }

            bool createMyAudiosDir(const DirPtr &parentDir, const std::string &dirName){
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
                    return true;
                } else {
                    return false;
                }
            }

        private:
            json parseJson(const std::string &str){
                auto res = json::parse(str);
                if(res.find("error") != res.end()) {
                    throw VkException("VK returned error response '" + str + "'");
                }
                return std::move(res);
            }

            std::string genFileName(const std::string &artist, const std::string &title) {
                return escapeName(artist + " - " + title);
            }

            std::string escapeName(std::string str) {
                boost::remove_erase_if(str, [](auto ch0) {
                    unsigned char ch = ch0;
                    return ch <= 31 || std::string("<>:/\\|?*").find(ch) != std::string::npos;
                });
                std::replace(str.begin(), str.end(), '"', '\'');
                return str;
            }
            std::string _ext;
            std::shared_ptr<TQueryMaker> _queryMaker;
            uint_fast32_t _numSearchFiles;
        };
    }
}