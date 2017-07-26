#ifndef PLUGKIT_STREAM_RESOLVER_HPP
#define PLUGKIT_STREAM_RESOLVER_HPP

#include <memory>
#include <string>

namespace plugkit {
class StreamResolver {
public:
  StreamResolver();
  ~StreamResolver();
  void resolve(std::pair<Layer *, std::string>* begin, size_t size);
  StreamResolver(const StreamResolver &) = delete;
  StreamResolver &operator=(const StreamResolver &) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
