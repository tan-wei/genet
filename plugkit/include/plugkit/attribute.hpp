#ifndef PLUGKIT_ATTRIBUTE_H
#define PLUGKIT_ATTRIBUTE_H

#include "variant.hpp"
#include "export.hpp"
#include "miniid.hpp"
#include <memory>

namespace plugkit {

class PLUGKIT_EXPORT Attribute final {
public:
  Attribute(miniid id, const Variant &value = Variant());
  ~Attribute();

  miniid id() const;
  void setId(miniid id);
  Variant value() const;
  void setValue(const Variant &value);

private:
  Attribute(const Attribute &attr) = delete;
  Attribute &operator=(const Attribute &attr) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};

using AttributeConstPtr = std::shared_ptr<Attribute>;
}

#endif
