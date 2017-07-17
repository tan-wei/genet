#ifndef PLUGKIT_LISTENER_H
#define PLUGKIT_LISTENER_H

#include "export.hpp"
#include "types.hpp"
#include <memory>
#include <v8.h>
#include <vector>

namespace plugkit {

class PLUGKIT_EXPORT Listener {
public:
  virtual ~Listener();
  virtual bool analyze(const FrameView *frame) = 0;
  virtual std::vector<Property *> properties() const;
  virtual std::vector<Chunk *> chunks() const;
};

using ListenerPtr = std::unique_ptr<Listener>;

class ListenerFactory;
using ListenerFactoryConstPtr = std::shared_ptr<const ListenerFactory>;

class Variant;

class PLUGKIT_EXPORT ListenerFactory {
public:
  virtual ~ListenerFactory();
  virtual ListenerPtr create(const Variant &args,
                             const SessionContext &context) const = 0;
  static v8::Local<v8::Object> wrap(const ListenerFactoryConstPtr &factory);
};
}

#endif
