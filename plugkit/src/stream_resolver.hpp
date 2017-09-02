#ifndef PLUGKIT_STREAM_RESOLVER_HPP
#define PLUGKIT_STREAM_RESOLVER_HPP

#include <memory>
#include <string>

namespace plugkit {

struct Layer;
struct Variant;

class StreamResolver final {
public:
  StreamResolver();
  ~StreamResolver();
  void resolve(std::pair<Layer *, Variant> *begin, size_t size);
  StreamResolver(const StreamResolver &) = delete;
  StreamResolver &operator=(const StreamResolver &) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
