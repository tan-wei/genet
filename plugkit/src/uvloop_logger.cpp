#include "uvloop_logger.hpp"
#include <iterator>
#include <mutex>
#include <uv.h>
#include <vector>

namespace plugkit {

class UvLoopLogger::Private {
public:
  uv_async_t async;
  std::mutex mutex;
  std::vector<MessagePtr> messages;
  Callback callback;
};

UvLoopLogger::UvLoopLogger(uv_loop_t *loop, const Callback &callback)
    : d(new Private()) {
  d->callback = callback;
  d->async.data = d;
  uv_async_init(loop, &d->async, [](uv_async_t *handle) {
    UvLoopLogger::Private *d =
        static_cast<UvLoopLogger::Private *>(handle->data);
    std::vector<MessagePtr> messages;
    {
      std::lock_guard<std::mutex> lock(d->mutex);
      messages.swap(d->messages);
    }
    for (MessagePtr &log : messages) {
      d->callback(std::move(log));
    }
  });
}

UvLoopLogger::~UvLoopLogger() {
  uv_close(reinterpret_cast<uv_handle_t *>(&d->async), [](uv_handle_t *handle) {
    delete static_cast<UvLoopLogger::Private *>(
        reinterpret_cast<uv_async_t *>(handle)->data);
  });
}

void UvLoopLogger::log(MessagePtr &&msg) {
  if (!msg)
    return;

  thread_local size_t previousHash = 0;
  if (msg->trivial) {
    size_t hash = std::hash<MessagePtr>{}(msg);
    if (hash == previousHash) {
      return;
    } else {
      previousHash = hash;
    }
  }

  thread_local std::vector<MessagePtr> messages;
  messages.push_back(std::move(msg));
  if (d->mutex.try_lock()) {
    std::move(messages.begin(), messages.end(),
              std::inserter(d->messages, d->messages.end()));
    d->mutex.unlock();
    uv_async_send(&d->async);
    messages.clear();
    previousHash = 0;
  }
}
}
