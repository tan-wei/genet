#include "listener_status.hpp"
#include "attribute.hpp"
#include <mutex>
#include <algorithm>
#include <vector>

namespace plugkit {

class ListenerStatus::Private {
public:
  std::mutex mutex;
  uint32_t revision = 0;
  std::vector<AttributeConstPtr> attributes;
  std::vector<ChunkConstPtr> chunks;
};

ListenerStatus::ListenerStatus() : d(new Private()) {}

ListenerStatus::~ListenerStatus() {}

uint32_t ListenerStatus::revision() const {
  std::lock_guard<std::mutex> lock(d->mutex);
  return d->revision;
}

size_t ListenerStatus::attributes() const {
  std::lock_guard<std::mutex> lock(d->mutex);
  return d->attributes.size();
}

size_t ListenerStatus::chunks() const {
  std::lock_guard<std::mutex> lock(d->mutex);
  return d->chunks.size();
}

AttributeConstPtr ListenerStatus::getAttribute(size_t index) const {
  std::lock_guard<std::mutex> lock(d->mutex);
  if (index >= d->attributes.size())
    return AttributeConstPtr();
  return d->attributes[index];
}

ChunkConstPtr ListenerStatus::getChunk(size_t index) const {
  std::lock_guard<std::mutex> lock(d->mutex);
  if (index >= d->chunks.size())
    return ChunkConstPtr();
  return d->chunks[index];
}

void ListenerStatus::addAttribute(const AttributeConstPtr &attr) {
  std::lock_guard<std::mutex> lock(d->mutex);
  miniid id = attr->id();
  auto it = std::find_if(
      d->attributes.begin(), d->attributes.end(),
      [id](const AttributeConstPtr &attr) { return attr->id() == id; });
  if (it != d->attributes.end()) {
    *it = attr;
  } else {
    d->attributes.push_back(attr);
  }
  ++d->revision;
}

void ListenerStatus::addChunk(const ChunkConstPtr &chunk) {
  std::lock_guard<std::mutex> lock(d->mutex);
  d->chunks.push_back(chunk);
  ++d->revision;
}
}
