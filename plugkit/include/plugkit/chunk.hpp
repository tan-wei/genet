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
  Chunk(const std::string &ns, const std::string &id, const Slice &payload);
  Chunk(Chunk &&chunk);
  ~Chunk();

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
  Chunk(const Chunk &layer) = delete;
  Chunk &operator=(const Chunk &layer) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
