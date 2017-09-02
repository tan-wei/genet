#include "stream_resolver.hpp"
#include "layer.hpp"
#include "variant.hpp"
#include <unordered_map>
#include <mutex>

namespace plugkit {

class StreamResolver::Private {
public:
  std::unordered_map<std::string, uint32_t> map;
  uint32_t counter = 0;
  std::mutex mutex;
};

StreamResolver::StreamResolver() : d(new Private()) {}

StreamResolver::~StreamResolver() {}

void StreamResolver::resolve(std::pair<Layer *, Variant> *begin, size_t size) {
  std::lock_guard<std::mutex> lock(d->mutex);
  for (size_t i = 0; i < size; ++i) {
    const auto &pair = begin[i];
    const std::string &data = pair.second.string();
    const auto it = d->map.find(data);
    if (it != d->map.end()) {
      pair.first->setStreamId(it->second);
    } else {
      d->map[data] = ++d->counter;
      pair.first->setStreamId(d->counter);
    }
  }
}
}
