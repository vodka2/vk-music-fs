#include "FileProcessor.h"

using namespace vk_music_fs;

FileProcessorInt::FileProcessorInt()
:
    _metadataWasRead(false), _bufferSizeLimitReached(false), _closed(false), _finished(false),
    _opened(false), _error(false),
    _openedPromise(std::make_shared<std::promise<void>>()),
    _threadPromise(std::make_shared<std::promise<void>>()),
    _bufferAppendPromise(std::make_shared<std::promise<void>>()),
    _openFuture(std::move(_openedPromise->get_future())),
    _threadFinishedFuture(std::move(_threadPromise->get_future())),
    _bufferAppendFuture(std::move(_bufferAppendPromise->get_future())){
}

void FileProcessorInt::waitForStopAppend() {
    _metadataWasRead = true;
    std::unique_lock<std::mutex> lock(_bufferAppendMutex);
    _bufferAppendFuture.get();
}

void FileProcessorInt::notifyAppendStopped() {
    _bufferAppendPromise->set_value();
}

bool FileProcessorInt::isFinished() {
    return _finished;
}
