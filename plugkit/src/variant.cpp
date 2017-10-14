#include "variant.hpp"
#include "plugkit_module.hpp"
#include <iomanip>
#include <nan.h>
#include <sstream>
#include <unordered_map>
#include <uv.h>

namespace plugkit {

namespace {

const Variant::Array nullArray;
const Variant::Map nullMap;
} // namespace

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

Variant::Variant(int8_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(uint8_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(int16_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(uint16_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(int32_t value) : type_(VARTYPE_INT32) { d.int_ = value; }

Variant::Variant(uint32_t value) : type_(VARTYPE_UINT32) { d.uint_ = value; }

Variant::Variant(int64_t value) : type_(VARTYPE_INT64) { d.int_ = value; }

Variant::Variant(uint64_t value) : type_(VARTYPE_UINT64) { d.uint_ = value; }

Variant::Variant(double value) : type_(VARTYPE_DOUBLE) { d.double_ = value; }

Variant::Variant(const std::string &str) : type_(VARTYPE_STRING) {
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

Variant::Variant(const Array &array) : type_(VARTYPE_ARRAY) {
  d.array = new Array(array);
}

Variant::Variant(const Map &map) : type_(VARTYPE_MAP) { d.map = new Map(map); }

Variant::Variant(const Slice &slice) : type_(VARTYPE_SLICE) {
  d.slice = new Slice(slice);
}

Variant::~Variant() {
  switch (type()) {
  case VARTYPE_STRING:
    if (tag() == 0) {
      delete d.str;
    }
    break;
  case VARTYPE_SLICE:
    delete d.slice;
    break;
  case VARTYPE_ARRAY:
    delete d.array;
    break;
  case VARTYPE_MAP:
    delete d.map;
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
  case VARTYPE_SLICE:
    this->d.slice = new Slice(*value.d.slice);
    break;
  case VARTYPE_ARRAY:
    this->d.array = new Array(*value.d.array);
    break;
  case VARTYPE_MAP:
    this->d.map = new Map(*value.d.map);
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
  case VARTYPE_INT32:
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT32:
  case VARTYPE_UINT64:
    return d.uint_;
  case VARTYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isInt32() const { return type() == VARTYPE_INT32; }

int32_t Variant::int32Value(int32_t defaultValue) const {
  switch (type()) {
  case VARTYPE_BOOL:
    return d.bool_;
  case VARTYPE_INT32:
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT32:
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
  case VARTYPE_INT32:
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT32:
  case VARTYPE_UINT64:
    return d.uint_;
  case VARTYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isUint32() const { return type() == VARTYPE_UINT32; }

uint32_t Variant::uint32Value(uint32_t defaultValue) const {
  switch (type()) {
  case VARTYPE_BOOL:
    return d.bool_;
  case VARTYPE_INT32:
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT32:
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
  case VARTYPE_INT32:
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT32:
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
  case VARTYPE_INT32:
  case VARTYPE_INT64:
    return d.int_;
  case VARTYPE_UINT32:
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
  case VARTYPE_BOOL:
    return boolValue() ? "true" : "false";
  case VARTYPE_INT32:
  case VARTYPE_INT64:
    return std::to_string(int64Value());
  case VARTYPE_UINT32:
  case VARTYPE_UINT64:
    return std::to_string(uint64Value());
  case VARTYPE_DOUBLE:
    return std::to_string(doubleValue());
  default:
    return defaultValue;
  }
}

Slice Variant::slice() const {
  if (isSlice()) {
    return *d.slice;
  } else {
    return Slice();
  }
}

const Variant::Array &Variant::array() const {
  if (type() == VARTYPE_ARRAY) {
    return *d.array;
  } else {
    return nullArray;
  }
}

const Variant::Map &Variant::map() const {
  if (type() == VARTYPE_MAP) {
    return *d.map;
  } else {
    return nullMap;
  }
}

Variant Variant::operator[](size_t index) const {
  if (type() == VARTYPE_ARRAY && index < d.array->size()) {
    return (*d.array)[index];
  } else {
    return Variant();
  }
}

Variant &Variant::operator[](size_t index) {
  if (type() == VARTYPE_ARRAY) {
    if (index >= d.array->size()) {
      d.array->resize(index + 1);
    }
    return (*d.array)[index];
  }
  static Variant null;
  return null;
}

Variant Variant::operator[](const std::string &key) const {
  if (type() == VARTYPE_MAP) {
    auto it = d.map->find(key);
    if (it != d.map->end()) {
      return it->second;
    }
  }
  return Variant();
}

Variant &Variant::operator[](const std::string &key) {
  if (type() == VARTYPE_MAP) {
    return (*d.map)[key];
  }
  static Variant null;
  return null;
}

size_t Variant::length() const {
  if (type() == VARTYPE_ARRAY) {
    return d.array->size();
  } else if (type() == VARTYPE_MAP) {
    return d.map->size();
  }
  return 0;
}

uint8_t Variant::tag() const { return type_ >> 4; }

bool Variant::isString() const { return type() == VARTYPE_STRING; }

bool Variant::isSlice() const { return type() == VARTYPE_SLICE; }

bool Variant::isArray() const { return type() == VARTYPE_ARRAY; }

bool Variant::isMap() const { return type() == VARTYPE_MAP; }

v8::Local<v8::Object> Variant::getNodeBuffer(const Slice &slice) {
  using namespace v8;

  Isolate *isolate = Isolate::GetCurrent();
  if (!isolate->GetData(1)) { // Node.js is not installed
    size_t sliceLen = Slice_length(slice);
    return v8::Uint8Array::New(
        v8::ArrayBuffer::New(isolate, const_cast<char *>(slice.begin),
                             sliceLen),
        0, sliceLen);
  }
  auto nodeBuf = node::Buffer::New(isolate, const_cast<char *>(slice.begin),
                                   Slice_length(slice),
                                   [](char *data, void *hint) {}, nullptr)
                     .ToLocalChecked();

  auto addr = Nan::New<v8::Array>(2);
  addr->Set(0,
            Nan::New(static_cast<uint32_t>(
                0xffffffff & (reinterpret_cast<uint64_t>(slice.begin) >> 32))));
  addr->Set(1, Nan::New(static_cast<uint32_t>(
                   0xffffffff & reinterpret_cast<uint64_t>(slice.begin))));
  nodeBuf->Set(Nan::New("addr").ToLocalChecked(), addr);
  return nodeBuf;
}

Slice Variant::getSlice(v8::Local<v8::ArrayBufferView> obj) {
  using namespace v8;
  auto buf = obj->Buffer();
  size_t len = obj->ByteLength();
  char *data =
      static_cast<char *>(buf->GetContents().Data()) + obj->ByteOffset();
  return Slice{data, data + len};
}

v8::Local<v8::Value> Variant::getValue(const Variant &var) {
  switch (var.type()) {
  case VARTYPE_BOOL:
    return Nan::New(var.boolValue());
  case VARTYPE_INT32:
    return Nan::New(var.int32Value());
  case VARTYPE_INT64:
    return Nan::New(std::to_string(var.int64Value())).ToLocalChecked();
  case VARTYPE_UINT32:
    return Nan::New(var.uint32Value());
  case VARTYPE_UINT64:
    return Nan::New(std::to_string(var.uint64Value())).ToLocalChecked();
  case VARTYPE_DOUBLE:
    return Nan::New(var.doubleValue());
  case VARTYPE_STRING:
    return Nan::New(var.string()).ToLocalChecked();
  case VARTYPE_ARRAY: {
    const auto &array = var.array();
    auto obj = Nan::New<v8::Array>(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
      obj->Set(i, getValue(array[i]));
    }
    return obj;
  }
  case VARTYPE_MAP: {
    const auto &map = var.map();
    auto obj = Nan::New<v8::Object>();
    for (const auto &pair : map) {
      obj->Set(Nan::New(pair.first).ToLocalChecked(), getValue(pair.second));
    }
    return obj;
  }
  case VARTYPE_SLICE:
    return getNodeBuffer(var.slice());
  default:
    return Nan::Null();
  }
}

json11::Json Variant::getJson(const Variant &var) {
  using namespace json11;
  Json::object json;
  switch (var.type()) {
  case VARTYPE_NIL:
    json["type"] = "nil";
    json["value"] = Json();
    break;
  case VARTYPE_BOOL:
    json["type"] = "bool";
    json["value"] = var.boolValue();
    break;
  case VARTYPE_INT32:
    json["type"] = "int32";
    json["value"] = var.doubleValue();
    break;
  case VARTYPE_UINT32:
    json["type"] = "uint32";
    json["value"] = var.doubleValue();
    break;
  case VARTYPE_INT64:
    json["type"] = "int64";
    json["value"] = var.string();
    break;
  case VARTYPE_UINT64:
    json["type"] = "uint64";
    json["value"] = var.string();
    break;
  case VARTYPE_DOUBLE:
    json["type"] = "double";
    json["value"] = var.doubleValue();
    break;
  case VARTYPE_STRING:
    json["type"] = "string";
    json["value"] = var.string();
    break;
  case VARTYPE_SLICE:
    json["type"] = "slice";
    {
      std::stringstream stream;
      stream << std::hex << std::setfill('0') << std::setw(2);
      const auto &slice = var.slice();
      const uint8_t *data = reinterpret_cast<const uint8_t *>(slice.begin);
      size_t length = Slice_length(slice);
      for (size_t i = 0; i < length; ++i) {
        stream << static_cast<int>(data[i]);
      }
      json["value"] = stream.str();
    }
    break;
  case VARTYPE_ARRAY:
    json["type"] = "array";
    {
      Json::array array;
      for (const auto &item : var.array()) {
        array.push_back(getJson(item));
      }
      json["value"] = array;
    }
    break;
  case VARTYPE_MAP:
    json["type"] = "map";
    {
      Json::object object;
      for (const auto &pair : var.map()) {
        object[pair.first] = getJson(pair.second);
      }
      json["value"] = object;
    }
    break;
  default:;
  }
  json["tag"] = var.tag();
  return json;
}

Variant Variant::getVariant(v8::Local<v8::Value> var) {
  if (var->IsBoolean()) {
    return var->BooleanValue();
  } else if (var->IsNumber()) {
    return var->NumberValue();
  } else if (var->IsString()) {
    return std::string(*Nan::Utf8String(var));
  } else if (var->IsArrayBufferView()) {
    return getSlice(var.As<v8::ArrayBufferView>());
  } else if (var->IsArray()) {
    auto obj = var.As<v8::Array>();
    Variant::Array array;
    array.resize(obj->Length());
    for (size_t i = 0; i < obj->Length(); ++i) {
      array[i] = getVariant(obj->Get(i));
    }
    return array;
  } else if (var->IsObject()) {
    auto obj = var.As<v8::Object>();
    auto keys = obj->GetOwnPropertyNames();
    Variant::Map map;
    for (size_t i = 0; i < keys->Length(); ++i) {
      const auto keyObj = keys->Get(i);
      const std::string &key = *Nan::Utf8String(keyObj);
      const Variant &value = getVariant(obj->Get(keyObj));
      map.emplace(key, value);
    }
    return map;
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

int32_t Variant_int32(const Variant *var) {
  if (!var)
    return 0;
  return var->int32Value();
}
void Variant_setInt32(Variant *var, int32_t value) { *var = Variant(value); }

int64_t Variant_int64(const Variant *var) {
  if (!var)
    return 0;
  return var->int64Value();
}
void Variant_setInt64(Variant *var, int64_t value) { *var = Variant(value); }

uint32_t Variant_uint32(const Variant *var) {
  if (!var)
    return 0;
  return var->uint32Value();
}
void Variant_setUint32(Variant *var, uint32_t value) { *var = Variant(value); }

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

const char *Variant_string(const Variant *var) {
  if (var && var->isString() && var->d.str) {
    if (var->tag() == 0) {
      return (*var->d.str)->c_str();
    } else {
      return reinterpret_cast<const char *>(&var->d.str);
    }
  }
  return "";
}
void Variant_setString(Variant *var, const char *str, int length) {
  *var = Variant(length < 0 ? std::string(str) : std::string(str, length));
}

Slice Variant_slice(const Variant *var) {
  if (var && var->isSlice()) {
    return var->slice();
  }
  return Slice{nullptr, nullptr};
}
void Variant_setSlice(Variant *var, Slice slice) { *var = Variant(slice); }

const Variant *Variant_arrayValue(const Variant *var, size_t index) {
  if (!var)
    return nullptr;
  const auto &array = var->array();
  if (index < array.size()) {
    return &array[index];
  }
  return nullptr;
}

Variant *Variant_arrayValueRef(Variant *var, size_t index) {
  if (!var->isArray()) {
    *var = Variant::Array();
  }
  return &(*var)[index];
}

const Variant *
Variant_mapValue(const Variant *var, const char *key, int length) {
  if (!var)
    return nullptr;
  if (var->isMap()) {
    const auto &map = var->map();
    auto it =
        map.find(length < 0 ? std::string(key) : std::string(key, length));
    if (it != map.end()) {
      return &it->second;
    }
  }
  return nullptr;
}

Variant *Variant_mapValueRef(Variant *var, const char *key, int length) {
  if (!var->isMap()) {
    *var = Variant::Map();
  }
  return &(*var)[length < 0 ? std::string(key) : std::string(key, length)];
}
} // namespace plugkit
