#pragma once

#include <mutex>
#include <thread>
#include <unordered_map>

namespace vk_music_fs {
    #define OVERRIDABLE_SETTING(T, D)                                    \
    class D##_cls {                                                      \
     private:                                                            \
        std::unordered_map<std::thread::id, T> localData;                \
        T globalData;                                                    \
        std::mutex _accessMutex;                                         \
     public:                                                             \
            D##_cls (T initialValue) {                                   \
                globalData = initialValue;                               \
            }                                                            \
            T get() {                                                    \
                std::lock_guard<std::mutex> lock{_accessMutex};          \
                if (localData.count(                                     \
                        std::this_thread::get_id()) != 0) {              \
                    return localData[std::this_thread::get_id()];        \
                } else {                                                 \
                    return globalData;                                   \
                }                                                        \
            }                                                            \
            void set(T newValue) {                                       \
                std::lock_guard<std::mutex> lock{_accessMutex};          \
                localData[std::this_thread::get_id()] = newValue;        \
            }                                                            \
            void clear() {                                               \
                std::lock_guard<std::mutex> lock{_accessMutex};          \
                localData.erase(std::this_thread::get_id());             \
            }                                                            \
            operator T() { return get(); }                               \
    };                                                                   \
    struct D {                                                           \
        using type = T;                                                  \
        std::shared_ptr<D##_cls> data;                                   \
        D(T value) { data = std::make_shared<D##_cls>(value); };         \
        operator T() { return data->get(); }                             \
    };                                                                   \


    template <typename T>
    class TempOverride {
    private:
        T _setting;
    public:
        TempOverride(T setting, typename T::type newValue) : _setting(setting){
            _setting.data->set(newValue);
        }

        ~TempOverride() {
            _setting.data->clear();
        }
    };
}
