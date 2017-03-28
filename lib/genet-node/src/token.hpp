#ifndef GENET_NODE_TOKEN_WRAPPER_H
#define GENET_NODE_TOKEN_WRAPPER_H

#include <nan.h>

namespace genet_node {
namespace Token {
void init(v8::Local<v8::Object> exports);
}
} // namespace genet_node

#endif