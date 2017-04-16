#ifndef PLUGKIT_CHUNK_H
#define PLUGKIT_CHUNK_H

#include <memory>
#include <vector>
#include <string>

namespace plugkit {

class Slice;

class Property;
using PropertyConstPtr = std::shared_ptr<const Property>;

class Chunk final {
public:
  Chunk();
  ~Chunk();
  Chunk(const Chunk &layer) = delete;
  Chunk &operator=(const Chunk &layer) = delete;

  std::string streamNs() const;
  void setStreamNs(const std::string &ns);
  std::string streamId() const;
  void setStreamId(const std::string &id);

  const Slice &payload() const;
  void setPayload(const Slice &payload);

  const std::vector<PropertyConstPtr> &properties() const;
  PropertyConstPtr propertyFromId(const std::string &id) const;
  void addProperty(const PropertyConstPtr &prop);

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
