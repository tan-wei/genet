#include "variant.hpp"
#include "plugkit_module.hpp"
#include <iomanip>
#include <nan.h>
#include <sstream>
#include <unordered_map>
#include <uv.h>

namespace plugkit {
void Variant::init(v8::Isolate *isolate) {
  PlugkitModule *module = PlugkitModule::get(isolate);

  auto script =
      Nan::CompileScript(Nan::New("(function (buf) { return "
                                  "require('buffer').Buffer.from(buf) })")
                             .ToLocalChecked());
  auto func = Nan::RunScript(script.ToLocalChecked())
                  .ToLocalChecked()
                  .As<v8::Function>();
  module->arrayToBuffer.Reset(isolate, func);
}

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

Variant::Variant(const char *data) : type_(VARTYPE_STRING) {
  const std::string &str = data;
  if (str.empty()) {
    d.str = nullptr;
  } else if (str.size() < sizeof(d.str)) {
    type_ = VARTYPE_STRING | (str.size() << 4);
    d.str = nullptr;
    str.copy(reinterpret_cast<char *>(&d.str), sizeof(d.str));
  } else {
    d.str = new std::shared_ptr<std::string>(new std::string(str));
  }
}

Variant::Variant(const char *str, size_t length) {
  type_ = VARTYPE_STRING_REF | (length << 4);
  d.data = str;
}

Variant::Variant(const Slice &slice) {
  type_ = VARTYPE_SLICE | (slice.length << 4);
  d.data = slice.data;
}

Variant Variant::fromAddress(void *ptr)
{
  Variant var;
  var.type_ = VARTYPE_ADDRESS;
  var.d.ptr = ptr;
  return var;
}

Variant::~Variant() {
  switch (type()) {
  case VARTYPE_STRING:
    if (tag() == 0) {
      delete d.str;
    }
    break;
  default:;
  }
}

Variant::Variant(const Variant &value) { *this = value; }

Variant &Variant::operator=(const Variant &value) {
  this->type_ = value.type_;
  this->d = value.d;
  switch (this->type()) {
  case VARTYPE_STRING:
    if (value.d.str && tag() == 0) {
      this->d.str = new std::shared_ptr<std::string>(*value.d.str);
    } else {
      this->d.str = value.d.str;
    }
    break;
  default:;
  }
  return *this;
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
    if (d.str) {
      if (tag() == 0) {
        return **d.str;
      } else {
        return std::string(reinterpret_cast<const char *>(&d.str), tag());
      }
    } else {
      return std::string();
    }
  case VARTYPE_STRING_REF:
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

void* Variant::address() const {
  if (isAddress()) {
    return d.ptr;
  } else {
    return nullptr;
  }
}

uint64_t Variant::tag() const { return type_ >> 4; }

bool Variant::isString() const { return type() == VARTYPE_STRING; }

bool Variant::isStringRef() const { return type() == VARTYPE_STRING_REF; }

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
  case VARTYPE_STRING_REF:
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
    return Variant(*Nan::Utf8String(var));
  } else if (var->IsArrayBufferView()) {
    return getSlice(var.As<v8::ArrayBufferView>());
  }
  return Variant();
}

VariantType Variant_type(const Variant *var) { return var->type(); }

void Variant_setNil(Variant *var) { *var = Variant(); }

bool Variant_bool(const Variant *var) {
  if (!var)
    return false;
  return var->boolValue();
}
void Variant_setBool(Variant *var, bool value) { *var = Variant(value); }

int64_t Variant_int64(const Variant *var) {
  if (!var)
    return 0;
  return var->int64Value();
}
void Variant_setInt64(Variant *var, int64_t value) { *var = Variant(value); }

uint64_t Variant_uint64(const Variant *var) {
  if (!var)
    return 0;
  return var->uint64Value();
}
void Variant_setUint64(Variant *var, uint64_t value) { *var = Variant(value); }

double Variant_double(const Variant *var) {
  if (!var)
    return 0.0;
  return var->doubleValue();
}
void Variant_setDouble(Variant *var, double value) { *var = Variant(value); }

const char *Variant_string(const Variant *var, size_t *len) {
  if (var) {
    if (var->isString() && var->d.str) {
      if (var->tag() == 0) {
        const auto &str = *var->d.str;
        *len = str->size();
        return str->c_str();
      } else {
        *len = var->tag();
        return reinterpret_cast<const char *>(&var->d.str);
      }
    } else if (var->isStringRef() && var->d.data) {
      *len = var->tag();
      return var->d.data;
    }
  }
  *len = 0;
  return "";
}

void Variant_setString(Variant *var, const char *data) { *var = Variant(data); }

void Variant_setStringRef(Variant *var, const char *data, int length) {
  *var = Variant(data, length);
}

Slice Variant_slice(const Variant *var) {
  if (var && var->isSlice()) {
    return var->slice();
  }
  return Slice{nullptr, 0};
}
void Variant_setSlice(Variant *var, Slice slice) { *var = Variant(slice); }
} // namespace plugkit
