#include "variant.hpp"
#include "plugkit_module.hpp"
#include <iomanip>
#include <nan.h>
#include <sstream>
#include <unordered_map>
#include <uv.h>

namespace plugkit {
Variant::Variant() : type_(VARTYPE_NIL) {}

Variant::Variant(bool value) : type_(VARTYPE_BOOL) { d.bool_ = value; }

Variant::Variant(int8_t value) : Variant(static_cast<int32_t>(value)) {}

Variant::Variant(uint8_t value) : Variant(static_cast<uint32_t>(value)) {}

Variant::Variant(int16_t value) : Variant(static_cast<int32_t>(value)) {}

Variant::Variant(uint16_t value) : Variant(static_cast<uint32_t>(value)) {}

Variant::Variant(int32_t value) : type_(VARTYPE_INT64) { d.int_ = value; }

Variant::Variant(uint32_t value) : type_(VARTYPE_UINT64) { d.uint_ = value; }

Variant::Variant(int64_t value) : type_(VARTYPE_INT64) { d.int_ = value; }

Variant::Variant(uint64_t value) : type_(VARTYPE_UINT64) { d.uint_ = value; }

Variant::Variant(double value) : type_(VARTYPE_DOUBLE) { d.double_ = value; }

Variant::Variant(const Slice &slice) {
  type_ = VARTYPE_SLICE | (slice.length << 4);
  d.data = slice.data;
}

Variant Variant::fromString(const char *str, size_t length) {
  Variant var;
  var.type_ = VARTYPE_STRING | (length << 4);
  var.d.data = str;
  return var;
}

Variant Variant::fromAddress(void *ptr) {
  Variant var;
  var.type_ = VARTYPE_ADDRESS;
  var.d.ptr = ptr;
  return var;
}

VariantType Variant::type() const {
  return static_cast<VariantType>(type_ & 0x0f);
}

bool Variant::isNil() const { return type() == VARTYPE_NIL; }

bool Variant::isBool() const { return type() == VARTYPE_BOOL; }

bool Variant::boolValue(bool defaultValue) const {
  switch (type()) {
  case VARTYPE_BOOL:
    return d.bool_;
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT64:
    return d.uint_;
  case VARTYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isInt64() const { return type() == VARTYPE_INT64; }

int64_t Variant::int64Value(int64_t defaultValue) const {
  switch (type()) {
  case VARTYPE_BOOL:
    return d.bool_;
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT64:
    return d.uint_;
  case VARTYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isUint64() const { return type() == VARTYPE_UINT64; }

uint64_t Variant::uint64Value(uint64_t defaultValue) const {
  switch (type()) {
  case VARTYPE_BOOL:
    return d.bool_;
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT64:
    return d.uint_;
  case VARTYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isDouble() const { return type() == VARTYPE_DOUBLE; }

double Variant::doubleValue(double defaultValue) const {
  switch (type()) {
  case VARTYPE_BOOL:
    return d.bool_;
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT64:
    return d.uint_;
  case VARTYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

std::string Variant::string(const std::string &defaultValue) const {
  switch (type()) {
  case VARTYPE_STRING:
    return std::string(d.data, tag());
  default:
    return defaultValue;
  }
}

Slice Variant::slice() const {
  if (isSlice()) {
    return Slice{d.data, tag()};
  } else {
    return Slice();
  }
}

void *Variant::address() const {
  if (isAddress()) {
    return d.ptr;
  } else {
    return nullptr;
  }
}

uint64_t Variant::tag() const { return type_ >> 4; }

bool Variant::isString() const { return type() == VARTYPE_STRING; }

bool Variant::isSlice() const { return type() == VARTYPE_SLICE; }

bool Variant::isAddress() const { return type() == VARTYPE_ADDRESS; }

v8::Local<v8::Object> Variant::getNodeBuffer(const Slice &slice) {
  using namespace v8;

  Isolate *isolate = Isolate::GetCurrent();
  if (!isolate->GetData(1)) { // Node.js is not installed
    return v8::Uint8Array::New(
        v8::ArrayBuffer::New(isolate, const_cast<char *>(slice.data),
                             slice.length),
        0, slice.length);
  }
  auto nodeBuf =
      node::Buffer::New(isolate, const_cast<char *>(slice.data), slice.length,
                        [](char *data, void *hint) {}, nullptr)
          .ToLocalChecked();

  auto addr = Nan::New<v8::Array>(2);
  addr->Set(0,
            Nan::New(static_cast<uint32_t>(
                0xffffffff & (reinterpret_cast<uint64_t>(slice.data) >> 32))));
  addr->Set(1, Nan::New(static_cast<uint32_t>(
                   0xffffffff & reinterpret_cast<uint64_t>(slice.data))));
  nodeBuf->Set(Nan::New("addr").ToLocalChecked(), addr);
  return nodeBuf;
}

Slice Variant::getSlice(v8::Local<v8::ArrayBufferView> obj) {
  using namespace v8;
  auto buf = obj->Buffer();
  size_t len = obj->ByteLength();
  char *data =
      static_cast<char *>(buf->GetContents().Data()) + obj->ByteOffset();
  return Slice{data, len};
}

v8::Local<v8::Value> Variant::getValue(const Variant &var) {
  switch (var.type()) {
  case VARTYPE_BOOL:
    return Nan::New(var.boolValue());
  case VARTYPE_INT64:
    return Nan::New(static_cast<double>(var.int64Value()));
  case VARTYPE_UINT64:
    return Nan::New(static_cast<double>(var.uint64Value()));
  case VARTYPE_DOUBLE:
    return Nan::New(var.doubleValue());
  case VARTYPE_STRING:
    return Nan::New(var.string()).ToLocalChecked();
  case VARTYPE_SLICE:
    return getNodeBuffer(var.slice());
  default:
    return Nan::Null();
  }
}

Variant Variant::getVariant(v8::Local<v8::Value> var) {
  if (var->IsBoolean()) {
    return var->BooleanValue();
  } else if (var->IsNumber()) {
    return var->NumberValue();
  } else if (var->IsString()) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    PlugkitModule *module = PlugkitModule::get(isolate);
    const auto &str = Nan::Utf8String(var);
    return Variant::fromString(module->stringPool.get(*str, str.length()), str.length());
  } else if (var->IsArrayBufferView()) {
    return getSlice(var.As<v8::ArrayBufferView>());
  }
  return Variant();
}
} // namespace plugkit
