#ifndef PLUGNODE_MODULE_HPP
#define PLUGNODE_MODULE_HPP

#include <nan.h>
#include <vector>

namespace genet_node {

class Module final {
 public:
  using FunctionTemplate = v8::UniquePersistent<v8::FunctionTemplate>;
  using Function = v8::UniquePersistent<v8::Function>;
  using Prototype = v8::UniquePersistent<v8::Value>;
  struct Class {
    Function ctor;
  };

  enum Slot {
    CLASS_ITER,
    CLASS_LAYER,
    CLASS_SESSION,
    CLASS_SESSION_PROFILE,
    CLASS_VIEW,
    CLASS_FRAME,
    CLASS_CONTEXT,
    CLASS_ATTR
  };

 public:
  static void init(v8::Local<v8::Object> exports);
  static void destroy();
  static Module& current();
  Class& get(Slot slot);

 private:
  Module();

 private:
  std::vector<Class> classes;
};

}  // namespace genet_node

#endif
