#pragma once

#include <fs/FsUtils.h>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <set>
#include <fs/Dir.h>

class FsHelper{
public:
    std::set<std::string> listContents(vk_music_fs::fs::DirPtr dir){
        std::set<std::string> keys;
        boost::copy(dir->getContents() | boost::adaptors::map_keys, std::inserter(keys, keys.begin()));
        return keys;
    }
};
