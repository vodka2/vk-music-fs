#include <regex>
#include <RemoteFile.h>
#include "FsUtils.h"
#include "Dir.h"
#include "File.h"
#include "FsException.h"
#include "Mp3FileName.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

std::vector<std::string> FsUtils::splitPath(std::string path) {
    if (path == "/" || path == "") {
        return {};
    }
    std::vector<std::string> ret;
    boost::trim_if(path, boost::is_any_of("/"));
    boost::split(ret, path, boost::is_any_of("/"));
    return ret;
}

FsPath FsUtils::findPath(
        const DirPtr &dir, const std::string &path, uint_fast32_t pathSize
) {
    return findPath(dir, path, {}, pathSize);
}

FsPath FsUtils::findPath(
        const DirPtr &dir, const std::string &path, const std::vector<FsPath> &locked, uint_fast32_t pathSize
) {
    auto parts = splitPath(path);
    DirOrFile curItem = dir;
    FsPath fsPath{parts, static_cast<int_fast16_t>(pathSize)};
    int_fast32_t limit = parts.size();
    if (limit < 0) {
        return std::move(fsPath);
    }
    int_fast32_t lockLimit = parts.size() - pathSize;
    int_fast32_t i = 0;
    if(!isPathElementLocked(curItem, locked)) {
        curItem.lock();
    }
    fsPath.add(curItem);
    for (const auto &part: parts) {
        if (i == limit) {
            break;
        }
        if (curItem.isDir() && curItem.dir()->hasItem(part)) {
            auto item = curItem.dir()->getItem(part);
            if(!isPathElementLocked(item, locked)) {
                item.lock();
            }
            fsPath.add(item);
            if(i <= lockLimit){
                auto remItem = fsPath.removeFirst();
                if(!isPathElementLocked(remItem, locked)){
                    remItem.unlock();
                }
            }
            curItem = item;
        } else {
            if(i <= lockLimit){
                auto remItem = fsPath.removeFirst();
                if(!isPathElementLocked(remItem, locked)){
                    remItem.unlock();
                }
            }
            break;
        }
        i++;
    }
    return std::move(fsPath);
}

FsUtils::FsUtils() {

}

std::vector<std::string> FsUtils::getEntries(const DirPtr &dir) {
    std::vector<std::string> ret;
    for (const auto &item : dir->getContents()) {
        ret.push_back(item.first);
    }
    return ret;
}

std::vector<std::string> FsUtils::getEntries(const DirPtr &rootDir, const std::string &path, const std::string &fullPath) {
    auto fsPath = findPath(rootDir, path);
    FsPathUnlocker unlocker(fsPath);
    if (!fsPath.isPathMatched() || !fsPath.getLast().isDir()) {
        throw FsException(fullPath + " is not a directory");
    }
    auto dir = fsPath.getLast().dir();
    return getEntries(dir);
}

std::string FsUtils::stripPathPrefix(std::string path, const std::string &prefix) {
    boost::trim_if(path, boost::is_any_of("/"));
    path = path.substr(prefix.length());
    boost::trim_if(path, boost::is_any_of("/"));
    return path;
}

FileOrDirMeta FsUtils::getMeta(const DirPtr &rootDir, const std::string &path) {
    auto fsPath = findPath(rootDir, path);
    FsPathUnlocker unlocker(fsPath);
    if (fsPath.isPathMatched()) {
        if (fsPath.getLast().isDir()) {
            return {FileOrDirMeta::Type::DIR_ENTRY, fsPath.getLast().dir()->getTime()};
        } else {
            return {FileOrDirMeta::Type::FILE_ENTRY, fsPath.getLast().file()->getTime()};
        }
    }
    return {FileOrDirMeta::Type::NOT_EXISTS, 0};
}

QueryParams FsUtils::parseQuery(const std::string &dirName) {
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

void FsUtils::addFilesToDir(
        const DirPtr &dir, const std::vector<RemoteFile> &files,
        const std::shared_ptr<IdGenerator> &idGenerator, const std::string &extension
) {
    auto curTime = dir->getNumFiles();
    for(const auto &file: files){
        Mp3FileName fname(file.getArtist(), file.getTitle(), extension);
        while (dir->hasItem(fname.getFilename())) {
            fname.increaseNumberSuffix();
        }
        auto newFile = std::make_shared<File>(
                fname.getFilename(),
                idGenerator->getNextId(),
                curTime,
                RemoteFile{file.getUri(), file.getOwnerId(), file.getFileId(), fname.getArtist(), fname.getTitle()},
                dir
        );
        dir->addItem(newFile);
        curTime++;
    }
}

RemoteFile FsUtils::getRemoteFile(FsPath &fsPath, const std::string &fullPath) {
    if(!fsPath.isPathMatched()){
        throw FsException("File does not exist " + fullPath);
    }
    if(!fsPath.getLast().isFile()){
        throw FsException("Not a file " + fullPath);
    }
    return std::get<RemoteFile>(*fsPath.getLast().file()->getExtra());
}

bool FsUtils::isPathElementLocked(const DirOrFile &element, const std::vector<FsPath> &locked) {
    for(const auto &eli : locked){
        for(const auto &elj: eli.cgetAll()){
            if(elj.getId() == element.getId()){
                return true;
            }
        }
    }
    return false;
}

void FsUtils::deleteAllFiles(const DirPtr &dir) {
    auto &contents = dir->getContents();
    for (auto it = contents.begin(); it != contents.end();) {
        if(it->second.isFile()) {
            it = contents.erase(it);
        } else {
            it++;
        }
    }
}

void FsUtils::limitItems(const DirPtr &dir, uint_fast32_t num, bool leaveDirs) {
    std::vector<DirOrFile> items;
    for(const auto &s: dir->getContents()){
        if(!leaveDirs || s.second.isFile()){
            items.push_back(s.second);
        }
    }
    if(items.size() >= num) {
        std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
            return a.getTime() > b.getTime();
        });
        for (auto it = items.cbegin(); it != items.begin() + (items.size() - num); it++) {
            dir->removeItem(it->getName());
        }
    }
}
