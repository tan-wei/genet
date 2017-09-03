#include "variant.hpp"
#include "plugkit_module.hpp"
#include "wrapper/layer.hpp"
#include <iomanip>
#include <nan.h>
#include <sstream>
#include <unordered_map>
#include <uv.h>

namespace plugkit {

namespace {

const Variant::Array nullArray;
const Variant::Map nullMap;
}

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

Variant::Variant() : type_(TYPE_NIL) {}

Variant::Variant(bool value) : type_(TYPE_BOOL) { d.bool_ = value; }

Variant::Variant(int8_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(uint8_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(int16_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(uint16_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(int32_t value) : type_(TYPE_INT32) { d.int_ = value; }

Variant::Variant(uint32_t value) : type_(TYPE_UINT32) { d.uint_ = value; }

Variant::Variant(int64_t value) : type_(TYPE_INT64) { d.int_ = value; }

Variant::Variant(uint64_t value) : type_(TYPE_UINT64) { d.uint_ = value; }

Variant::Variant(double value) : type_(TYPE_DOUBLE) { d.double_ = value; }

Variant::Variant(const std::string &str) : type_(TYPE_STRING) {
  d.str = new std::shared_ptr<std::string>(new std::string(str));
}

Variant::Variant(std::string &&str) : type_(TYPE_STRING) {
  d.str = new std::shared_ptr<std::string>(new std::string(str));
}

Variant::Variant(const Array &array) : type_(TYPE_ARRAY) {
  d.array = new Array(array);
}

Variant::Variant(const Map &map) : type_(TYPE_MAP) { d.map = new Map(map); }

Variant::Variant(Array &&array) : type_(TYPE_ARRAY) {
  d.array = new Array(array);
}

Variant::Variant(Map &&map) : type_(TYPE_MAP) { d.map = new Map(map); }

Variant::Variant(const Slice &slice) : type_(TYPE_SLICE) {
  d.slice = new Slice(slice);
}

Variant::Variant(const Timestamp &ts) : type_(TYPE_TIMESTAMP) {
  d.ts = new Timestamp(ts);
}

Variant::Variant(const Layer *layer) : type_(TYPE_LAYER) { d.layer = layer; }

Variant::~Variant() {
  switch (type()) {
  case Variant::TYPE_TIMESTAMP:
    delete d.ts;
    break;
  case Variant::TYPE_STRING:
    delete d.str;
    break;
  case Variant::TYPE_SLICE:
    delete d.slice;
    break;
  case Variant::TYPE_ARRAY:
    delete d.array;
    break;
  case Variant::TYPE_MAP:
    delete d.map;
    break;
  default:;
  }
}

Variant::Variant(const Variant &value) { *this = value; }

Variant &Variant::operator=(const Variant &value) {
  this->type_ = value.type_;
  this->d = value.d;
  switch (this->type_) {
  case Variant::TYPE_TIMESTAMP:
    this->d.ts = new Timestamp(*value.d.ts);
    break;
  case Variant::TYPE_STRING:
    this->d.str = new std::shared_ptr<std::string>(*value.d.str);
    break;
  case Variant::TYPE_SLICE:
    this->d.slice = new Slice(*value.d.slice);
    break;
  case Variant::TYPE_ARRAY:
    this->d.array = new Array(*value.d.array);
    break;
  case Variant::TYPE_MAP:
    this->d.map = new Map(*value.d.map);
    break;
  default:;
  }
  return *this;
}

Variant::Type Variant::type() const { return static_cast<Type>(type_); }

bool Variant::isNil() const { return type() == TYPE_NIL; }

bool Variant::isBool() const { return type() == TYPE_BOOL; }

bool Variant::boolValue(bool defaultValue) const {
  switch (type_) {
  case TYPE_BOOL:
    return d.bool_;
  case TYPE_INT32:
  case TYPE_INT64:
    return d.int_;
  case TYPE_UINT32:
  case TYPE_UINT64:
    return d.uint_;
  case TYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isInt32() const { return type() == TYPE_INT32; }

int32_t Variant::int32Value(int32_t defaultValue) const {
  switch (type_) {
  case TYPE_BOOL:
    return d.bool_;
  case TYPE_INT32:
  case TYPE_INT64:
    return d.int_;
  case TYPE_UINT32:
  case TYPE_UINT64:
    return d.uint_;
  case TYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isInt64() const { return type() == TYPE_INT64; }

int64_t Variant::int64Value(int64_t defaultValue) const {
  switch (type_) {
  case TYPE_BOOL:
    return d.bool_;
  case TYPE_INT32:
  case TYPE_INT64:
    return d.int_;
  case TYPE_UINT32:
  case TYPE_UINT64:
    return d.uint_;
  case TYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isUint32() const { return type() == TYPE_UINT32; }

uint32_t Variant::uint32Value(uint32_t defaultValue) const {
  switch (type_) {
  case TYPE_BOOL:
    return d.bool_;
  case TYPE_INT32:
  case TYPE_INT64:
    return d.int_;
  case TYPE_UINT32:
  case TYPE_UINT64:
    return d.uint_;
  case TYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isUint64() const { return type() == TYPE_UINT64; }

uint64_t Variant::uint64Value(uint64_t defaultValue) const {
  switch (type_) {
  case TYPE_BOOL:
    return d.bool_;
  case TYPE_INT32:
  case TYPE_INT64:
    return d.int_;
  case TYPE_UINT32:
  case TYPE_UINT64:
    return d.uint_;
  case TYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

bool Variant::isDouble() const { return type() == TYPE_DOUBLE; }

double Variant::doubleValue(double defaultValue) const {
  switch (type_) {
  case TYPE_BOOL:
    return d.bool_;
  case TYPE_INT32:
  case TYPE_INT64:
    return d.int_;
  case TYPE_UINT32:
  case TYPE_UINT64:
    return d.uint_;
  case TYPE_DOUBLE:
    return d.double_;
  default:
    return defaultValue;
  }
}

std::string Variant::string(const std::string &defaultValue) const {
  switch (type()) {
  case TYPE_STRING:
    return **d.str;
  case TYPE_BOOL:
    return boolValue() ? "true" : "false";
  case TYPE_INT32:
  case TYPE_INT64:
    return std::to_string(int64Value());
  case TYPE_UINT32:
  case TYPE_UINT64:
    return std::to_string(uint64Value());
  case TYPE_DOUBLE:
    return std::to_string(doubleValue());
  case TYPE_TIMESTAMP: {
    auto tp = std::chrono::time_point_cast<std::chrono::seconds>(timestamp());
    std::time_t ts = std::chrono::system_clock::to_time_t(tp);
    std::stringstream stream;
    stream << std::put_time(std::localtime(&ts), "%FT%T%z");
    return stream.str();
  }
  default:
    return defaultValue;
  }
}

Timestamp Variant::timestamp(const Timestamp &defaultValue) const {
  if (isTimestamp()) {
    return *d.ts;
  } else {
    return defaultValue;
  }
}

const Layer *Variant::layer() const {
  if (isLayer()) {
    return d.layer;
  } else {
    return nullptr;
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
  if (type() == TYPE_ARRAY) {
    return *d.array;
  } else {
    return nullArray;
  }
}

const Variant::Map &Variant::map() const {
  if (type() == TYPE_MAP) {
    return *d.map;
  } else {
    return nullMap;
  }
}

Variant Variant::operator[](size_t index) const {
  if (type() == TYPE_ARRAY && index < d.array->size()) {
    return (*d.array)[index];
  } else {
    return Variant();
  }
}

Variant &Variant::operator[](size_t index) {
  if (type() == TYPE_ARRAY) {
    if (index >= d.array->size()) {
      d.array->resize(index + 1);
    }
    return (*d.array)[index];
  }
  static Variant null;
  return null;
}

Variant Variant::operator[](const std::string &key) const {
  if (type() == TYPE_MAP) {
    auto it = d.map->find(key);
    if (it != d.map->end()) {
      return it->second;
    }
  }
  return Variant();
}

Variant &Variant::operator[](const std::string &key) {
  if (type() == TYPE_MAP) {
    return (*d.map)[key];
  }
  static Variant null;
  return null;
}

size_t Variant::length() const {
  if (type() == TYPE_ARRAY) {
    return d.array->size();
  } else if (type() == TYPE_MAP) {
    return d.map->size();
  }
  return 0;
}

uint8_t Variant::tag() const { return tag_; }

bool Variant::isString() const { return type() == TYPE_STRING; }

bool Variant::isTimestamp() const { return type() == TYPE_TIMESTAMP; }

bool Variant::isLayer() const { return type() == TYPE_LAYER; }

bool Variant::isSlice() const { return type() == TYPE_SLICE; }

bool Variant::isArray() const { return type() == TYPE_ARRAY; }

bool Variant::isMap() const { return type() == TYPE_MAP; }

v8::Local<v8::Object> Variant::getNodeBuffer(const Slice &slice) {
  using namespace v8;

  Isolate *isolate = Isolate::GetCurrent();
  if (!isolate->GetData(1)) { // Node.js is not installed
    return v8::ArrayBuffer::New(isolate, const_cast<char *>(slice.begin),
                                Slice_length(slice));
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

Slice Variant::getSlice(v8::Local<v8::Object> obj) {
  using namespace v8;

  if (!node::Buffer::HasInstance(obj)) {
    return Slice();
  }

  size_t len = node::Buffer::Length(obj);
  char *data = new char[len];
  std::memcpy(data, node::Buffer::Data(obj), len);

  return Slice{data, data + len};
}

v8::Local<v8::Value> Variant::getValue(const Variant &var) {
  switch (var.type()) {
  case TYPE_BOOL:
    return Nan::New(var.boolValue());
  case TYPE_INT32:
    return Nan::New(var.int32Value());
  case TYPE_INT64:
    return Nan::New(std::to_string(var.int64Value())).ToLocalChecked();
  case TYPE_UINT32:
    return Nan::New(var.uint32Value());
  case TYPE_UINT64:
    return Nan::New(std::to_string(var.uint64Value())).ToLocalChecked();
  case TYPE_DOUBLE:
    return Nan::New(var.doubleValue());
  case TYPE_STRING:
    return Nan::New(var.string()).ToLocalChecked();
  case TYPE_TIMESTAMP: {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    var.timestamp().time_since_epoch())
                    .count();
    auto date = v8::Date::New(isolate, nano / 1000000.0).As<v8::Object>();
    date->Set(Nan::New("nsec").ToLocalChecked(),
              Nan::New(static_cast<double>(nano % 1000000)));
    return date;
  }
  case TYPE_LAYER:
    return LayerWrapper::wrap(var.layer());
  case TYPE_ARRAY: {
    const auto &array = var.array();
    auto obj = Nan::New<v8::Array>(array.size());
    for (size_t i = 0; i < array.size(); ++i) {
      obj->Set(i, getValue(array[i]));
    }
    return obj;
  }
  case TYPE_MAP: {
    const auto &map = var.map();
    auto obj = Nan::New<v8::Object>();
    for (const auto &pair : map) {
      obj->Set(Nan::New(pair.first).ToLocalChecked(), getValue(pair.second));
    }
    return obj;
  }
  case TYPE_SLICE:
    return getNodeBuffer(var.slice());
  default:
    return Nan::Null();
  }
}

json11::Json Variant::getJson(const Variant &var) {
  using namespace json11;
  Json::object json;
  switch (var.type()) {
  case Variant::TYPE_NIL:
    json["type"] = "nil";
    json["value"] = Json();
    break;
  case Variant::TYPE_BOOL:
    json["type"] = "bool";
    json["value"] = var.boolValue();
    break;
  case Variant::TYPE_INT32:
    json["type"] = "int32";
    json["value"] = var.doubleValue();
    break;
  case Variant::TYPE_UINT32:
    json["type"] = "uint32";
    json["value"] = var.doubleValue();
    break;
  case Variant::TYPE_INT64:
    json["type"] = "int64";
    json["value"] = var.string();
    break;
  case Variant::TYPE_UINT64:
    json["type"] = "uint64";
    json["value"] = var.string();
    break;
  case Variant::TYPE_DOUBLE:
    json["type"] = "double";
    json["value"] = var.doubleValue();
    break;
  case Variant::TYPE_STRING:
    json["type"] = "string";
    json["value"] = var.string();
    break;
  case Variant::TYPE_TIMESTAMP:
    json["type"] = "timestamp";
    json["value"] = var.string();
    break;
  case Variant::TYPE_LAYER:
    json["type"] = "layer";
    break;
  case Variant::TYPE_SLICE:
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
  case Variant::TYPE_ARRAY:
    json["type"] = "array";
    {
      Json::array array;
      for (const auto &item : var.array()) {
        array.push_back(getJson(item));
      }
      json["value"] = array;
    }
    break;
  case Variant::TYPE_MAP:
    json["type"] = "map";
    {
      Json::object object;
      for (const auto &pair : var.map()) {
        object[pair.first] = getJson(pair.second);
      }
      json["value"] = object;
    }
    break;
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
  } else if (var->IsDate()) {
    auto nsec = var.As<v8::Object>()->Get(Nan::New("nsec").ToLocalChecked());
    uint64_t ts =
        static_cast<uint64_t>(var.As<v8::Date>()->ValueOf()) * 1000000;
    if (nsec->IsNumber()) {
      ts += var->NumberValue();
    }
    return Timestamp(std::chrono::nanoseconds(ts));
  } else if (node::Buffer::HasInstance(var)) {
    return getSlice(var.As<v8::Object>());
  } else if (const Layer *layer =
                 LayerWrapper::unwrapConst(var.As<v8::Object>())) {
    return layer;
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
  if (var && var->isString()) {
    return (*var->d.str)->c_str();
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

const Variant *Variant_mapValue(const Variant *var, const char *key,
                                int length) {
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
}
