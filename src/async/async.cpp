#include "async.h"

#include <ctime>
#include <map>
#include <memory>
#include <mutex>

#include "async_command_processor.h"
#include "async_context.h"
#include "async_runtime.h"

namespace {

std::time_t current_time() { return std::time(nullptr); }

AsyncContext* to_context(async::handle_t handle) {
    return static_cast<AsyncContext*>(handle);
}

std::shared_ptr<AsyncCommandProcessor> get_command_processor(std::size_t block_size) {
    static std::mutex processors_mutex;
    static std::map<std::size_t, std::weak_ptr<AsyncCommandProcessor>> processors;

    std::lock_guard<std::mutex> lock(processors_mutex);
    std::shared_ptr<AsyncCommandProcessor> processor = processors[block_size].lock();
    if (!processor) {
        processor = std::make_shared<AsyncCommandProcessor>(
            block_size,
            [](const CommandBlock& block) { AsyncRuntime::instance().publish(block); });
        processors[block_size] = processor;
    }
    return processor;
}

}  // namespace

namespace async {

handle_t connect(std::size_t bulk) {
    return new AsyncContext(get_command_processor(bulk), current_time);
}

void receive(handle_t handle, const char* data, std::size_t size) {
    AsyncContext* context = to_context(handle);
    if (context == nullptr) {
        return;
    }

    context->receive(data, size);
}

void disconnect(handle_t handle) {
    AsyncContext* context = to_context(handle);
    if (context == nullptr) {
        return;
    }

    context->disconnect();
    delete context;

    AsyncRuntime::instance().wait();
}

}  // namespace async
