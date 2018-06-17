#ifndef PLUGNODE_SCRIPT_H
#define PLUGNODE_SCRIPT_H

#include <memory>
#include <nan.h>

namespace dyr_node {

class ArrayBufferAllocator;

class Script {
 public:
  Script();
  virtual ~Script();

 protected:
  v8::Isolate* isolate;
};

}  // namespace dyr_node

#endif
