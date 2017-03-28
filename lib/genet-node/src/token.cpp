#include "token.hpp"
#include "exports.hpp"

namespace {
NAN_METHOD(Token_get_wrap) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  Token token = genet_token_get(*Nan::Utf8String(info[0]));
  info.GetReturnValue().Set(token);
}

NAN_METHOD(Token_string_wrap) {
  if (!info[0]->IsUint32()) {
    Nan::ThrowTypeError("First argument must be a number");
    return;
  }
  auto str =
      Nan::New(genet_token_string(info[0]->Uint32Value())).ToLocalChecked();
  info.GetReturnValue().Set(str);
}
} // namespace

namespace genet_node {
namespace Token {
void init(v8::Local<v8::Object> exports) {
  auto token = Nan::New<v8::Object>();
  token->Set(Nan::New("get").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_get_wrap)->GetFunction());
  token->Set(Nan::New("string").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_string_wrap)->GetFunction());
  exports->Set(Nan::New("Token").ToLocalChecked(), token);
}
} // namespace Token
} // namespace genet_node
