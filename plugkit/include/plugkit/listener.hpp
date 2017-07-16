#ifndef PLUGKIT_LISTENER_H
#define PLUGKIT_LISTENER_H

#include "export.hpp"
#include "strns.hpp"
#include "property.hpp"
#include "chunk.hpp"
#include <memory>
#include <v8.h>
#include <vector>

namespace plugkit {

class FrameView;
struct SessionContext;

class PLUGKIT_EXPORT Listener {
public:
  virtual ~Listener();
  virtual bool analyze(const FrameView *frame) = 0;
  virtual std::vector<Property *> properties() = 0;
  virtual std::vector<Chunk *> chunks() = 0;
};

using DissectorPtr = std::unique_ptr<Dissector>;

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class PLUGKIT_EXPORT DissectorFactory {
public:
  virtual ~DissectorFactory();
  virtual DissectorPtr create(const SessionContext &context) const = 0;
  static v8::Local<v8::Object> wrap(const DissectorFactoryConstPtr &factory);
};
}

#endif
