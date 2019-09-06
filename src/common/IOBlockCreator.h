#pragma once

#include "IOBlock.h"
#include <memory>
#include <boost/pool/singleton_pool.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

namespace vk_music_fs {
    struct IOBlockTag {};
    template <typename TBlock>
    class IOBlockCreator {
    public:
        template <typename... T>
        IOBlockCreator(T&&... args){} //NOLINT
        struct PtrData: public boost::intrusive_ref_counter<PtrData>, public IOBlockWrap<TBlock> {
            PtrData(): IOBlockWrap<TBlock>{TBlock{}}{}
            static void operator delete (void *p) {
                boost::singleton_pool<IOBlockTag, sizeof(PtrData)>::free(p);
            }
        };
        using BlockPtr = boost::intrusive_ptr<PtrData>;
        BlockPtr create() {
            return BlockPtr {
                new (boost::singleton_pool<IOBlockTag, sizeof(PtrData)>::malloc()) PtrData()
            };
        }
    };
}
