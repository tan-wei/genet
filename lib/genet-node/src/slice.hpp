#ifndef GENET_NODE_SLICE_WRAPPER_H
#define GENET_NODE_SLICE_WRAPPER_H

#include <nan.h>

namespace genet_node {
namespace Slice {
void init(v8::Local<v8::Object> exports);
}
} // namespace genet_node

#endif