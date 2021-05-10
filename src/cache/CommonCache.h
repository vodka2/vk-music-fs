#pragma once

#include <net/Mp3SizeObtainer.h>
#include <common/common.h>
#include <common/RemotePhotoFile.h>
#include <mp3core/RemoteFile.h>
#include <mp3core/TagSizeCalculator.h>
#include <lrucache.hpp>
#include <mutex>
#include <unordered_set>
#include "CacheSaver.h"

namespace vk_music_fs {
    class Mp3FileSizeHelper {
    public:
        explicit Mp3FileSizeHelper(const std::shared_ptr<TagSizeCalculator> &tagSizeCalc): _tagSizeCalc(tagSizeCalc) {}
        uint_fast32_t getExtraSize(const RemoteFile &file) {
            return _tagSizeCalc->getTagSize(file);
        }
        TotalPrepSizes getZeroSize() {
            return TotalPrepSizes{0, 0};
        }
    private:
        std::shared_ptr<TagSizeCalculator> _tagSizeCalc;
    };

    class DummyFileSizeHelper {
    public:
        template <typename T>
        uint_fast32_t getExtraSize(T file) {
            return 0;
        }
        TotalSize getZeroSize() {
            return TotalSize{0};
        }
    };

    template <typename TFile, typename TInitialSize, typename TSizeHelper>
    class CommonCache {
    public:
        typedef typename TFile::IdType TId;
        typedef typename TId::Hasher THasher;
        CommonCache(
                const std::shared_ptr<net::Mp3SizeObtainer> &sizeObtainer,
                const std::shared_ptr<CacheSaver> &cacheSaver,
                const std::shared_ptr<TSizeHelper> &sizeHelper,
                SizesCacheSize sizesCacheSize,
                FilesCacheSize filesCacheSize
        ): _sizeObtainer(sizeObtainer), _cacheSaver(cacheSaver), _sizeHelper(sizeHelper),
        _sizesCache(sizesCacheSize, [](...) -> bool{return true;}),
        _initialSizesCache(filesCacheSize, [this](auto file) -> bool{
            if(_openedFiles.find(file) == _openedFiles.end()) {
                boost::filesystem::remove(_cacheSaver->constructFilename(file));
                return true;
            } else {
                return false;
            }
        }) {
            _cacheSaver->loadSizesFromFile<TId>([this] (auto ...args) {_sizesCache.put(args...);});
            _cacheSaver->loadInitialSizesFromFile<TId>([this] (auto ...args) {_initialSizesCache.put(args...);});
        }

        ~CommonCache() {
            _cacheSaver->saveSizesToFile<TId>([this] () {return _sizesCache.getList();});
            _cacheSaver->saveInitialSizesToFile<TId>([this] () {return _initialSizesCache.getList();});
        }

        void removeSize(const TId &fileId) {
            std::scoped_lock<std::mutex> lock(_sizesMutex);
            _sizesCache.remove(fileId);
        }

        FNameCache getFilename(const TFile &file) {
            std::scoped_lock<std::mutex> lock(_initialSizesMutex);
            auto id = TId{file};
            _openedFiles.insert(id);
            if(_initialSizesCache.exists(id) && boost::filesystem::exists(_cacheSaver->constructFilename(id))){
                return {_cacheSaver->constructFilename(id), (_initialSizesCache.get(id).totalSize == getFileSize(file))};
            } else {
                std::string name = _cacheSaver->constructFilename(id);
                _initialSizesCache.put(id, _sizeHelper->getZeroSize());
                return {name, false};
            }
        }

        uint_fast32_t getFileSize(const TFile &file) {
            while (true) {
                bool isObtainingSize = false;
                {
                    std::scoped_lock<std::mutex> lock(_sizesMutex);
                    if (_sizesCache.exists(file.getId())) {
                        auto valInCache = _sizesCache.get(file.getId());
                        if (valInCache != OBTAINING_FILE_SIZE) {
                            return _sizesCache.get(file.getId());
                        } else {
                            isObtainingSize = true;
                        }
                    } else {
                        _sizesCache.put(file.getId(), OBTAINING_FILE_SIZE);
                    }
                }
                if (isObtainingSize) {
                    std::unique_lock<std::mutex> lock(_sizesCondVarMutex);
                    _sizesCondVar.wait(lock);
                } else {
                    uint_fast32_t size;
                    try {
                        size = _sizeObtainer->getSize(file.getUri()) + _sizeHelper->getExtraSize(file);
                    } catch (...) {
                        {
                            std::scoped_lock<std::mutex> lock(_sizesMutex);
                            _sizesCache.remove(file.getId());
                        }
                        _sizesCondVar.notify_all();
                        throw;
                    }
                    {
                        std::scoped_lock<std::mutex> lock(_sizesMutex);
                        _sizesCache.put(file.getId(), size);
                    }
                    _sizesCondVar.notify_all();
                    return size;
                }
            }
        }

        uint_fast32_t getExtraSize(const TFile &file) {
            return _sizeHelper->getExtraSize(file);
        }

        uint_fast32_t getUriSize(const TFile &file) {
            return getFileSize(file) - _sizeHelper->getExtraSize(file);
        }

        TInitialSize getInitialSize(const TId &file) {
            std::scoped_lock<std::mutex> lock(_initialSizesMutex);
            return _initialSizesCache.get(file);
        }

        void fileClosed(const TFile &file, const TInitialSize &size) {
            std::scoped_lock<std::mutex> lock(_initialSizesMutex);
            _openedFiles.erase(file.getId());
            _initialSizesCache.put(file.getId(), size);
        }

    private:
        static constexpr int_fast32_t OBTAINING_FILE_SIZE = -1;
        std::mutex _initialSizesMutex;
        std::mutex _sizesMutex;
        std::mutex _sizesCondVarMutex;
        std::condition_variable _sizesCondVar;
        std::shared_ptr<CacheSaver> _cacheSaver;
        std::shared_ptr<net::Mp3SizeObtainer> _sizeObtainer;
        cache::lru_cache<TId, int_fast32_t, THasher> _sizesCache;
        cache::lru_cache<TId, TInitialSize, THasher> _initialSizesCache;
        std::unordered_set<TId, THasher> _openedFiles;
        std::shared_ptr<TSizeHelper> _sizeHelper;
    };
}
