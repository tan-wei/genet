#include "private/variant.hpp"
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

struct SharedBufferStore {
  uv_rwlock_t rwlock;
  std::unordered_map<const char *, std::weak_ptr<std::string>> map;

  SharedBufferStore() { uv_rwlock_init(&rwlock); }

  ~SharedBufferStore() { uv_rwlock_destroy(&rwlock); }

  std::shared_ptr<std::string> find(const char *addr) {
    uv_rwlock_rdlock(&rwlock);
    std::shared_ptr<std::string> buf;
    auto it = map.find(addr);
    if (it != map.end()) {
      buf = it->second.lock();
    }
    uv_rwlock_rdunlock(&rwlock);
    return buf;
  }

  void add(const std::shared_ptr<std::string> &buf) {
    uv_rwlock_wrlock(&rwlock);
    map[buf->data()] = buf;
    uv_rwlock_wrunlock(&rwlock);
  }

  void cleanup() {
    uv_rwlock_wrlock(&rwlock);
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (!it->second.lock()) {
        it = map.erase(it);
      }
    }
    uv_rwlock_wrunlock(&rwlock);
  }
};

SharedBufferStore bufferStore;
}

void Variant::Private::init(v8::Isolate *isolate) {
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

Variant::Variant(int32_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(uint32_t value) : Variant(static_cast<double>(value)) {}

Variant::Variant(int64_t value) : type_(TYPE_INT64) { d.int_ = value; }

Variant::Variant(uint64_t value) : type_(TYPE_UINT64) { d.uint_ = value; }

Variant::Variant(double value) : type_(TYPE_DOUBLE) { d.double_ = value; }

Variant::Variant(const std::string &str) : type_(TYPE_STRING) {
  d.str = new std::string(str);
}

Variant::Variant(std::string &&str) : type_(TYPE_STRING) {
  d.str = new std::string(str);
}

Variant::Variant(const Array &array) : type_(TYPE_ARRAY) {
  d.array = new Array(array);
}

Variant::Variant(const Map &map) : type_(TYPE_MAP) { d.map = new Map(map); }

Variant::Variant(Array &&array) : type_(TYPE_ARRAY) {
  d.array = new Array(array);
}

Variant::Variant(Map &&map) : type_(TYPE_MAP) { d.map = new Map(map); }

Variant::Variant(const View &view) : type_(TYPE_VIEW) {
  d.view = new View(view);
}

Variant::Variant(const Timestamp &ts) : type_(TYPE_TIMESTAMP) {
  d.ts = new Timestamp(ts);
}

Variant::~Variant() {
  switch (type()) {
  case Variant::TYPE_TIMESTAMP:
    delete d.ts;
    break;
  case Variant::TYPE_STRING:
    delete d.str;
    break;
  case Variant::TYPE_VIEW:
    delete d.view;
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
    this->d.str = new std::string(*value.d.str);
    break;
  case Variant::TYPE_VIEW:
    this->d.view = new View(*value.d.view);
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

void Variant::Private::cleanupSharedBuffers() { bufferStore.cleanup(); }

Variant::Type Variant::type() const { return static_cast<Type>(type_); }

bool Variant::isNil() const { return type() == TYPE_NIL; }

bool Variant::isBool() const { return type() == TYPE_BOOL; }

bool Variant::boolValue(bool defaultValue) const {
  switch (type_) {
  case TYPE_BOOL:
    return d.bool_;
  case TYPE_INT64:
    return d.int_;
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
  case TYPE_INT64:
    return d.int_;
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
  case TYPE_INT64:
    return d.int_;
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
  case TYPE_INT64:
    return d.int_;
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
    return d.str->c_str();
  case TYPE_BOOL:
    return boolValue() ? "true" : "false";
  case TYPE_INT64:
    return std::to_string(int64Value());
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

View Variant::view() const {
  if (isView()) {
    return *d.view;
  } else {
    return View();
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

Variant Variant::operator[](const std::string &key) const {
  if (type() == TYPE_MAP) {
    for (const auto &pair : *d.map) {
      if (pair.first == key) {
        return pair.second;
      }
    }
  }
  return Variant();
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

bool Variant::isView() const { return type() == TYPE_VIEW; }

v8::Local<v8::Object> Variant::Private::getNodeBuffer(const View &view) {
  using namespace v8;

  Isolate *isolate = Isolate::GetCurrent();
  if (!isolate->GetData(1)) { // Node.js is not installed
    return v8::ArrayBuffer::New(isolate, const_cast<char *>(view.begin),
                                View_length(view));
  }
  auto nodeBuf = node::Buffer::New(isolate, const_cast<char *>(view.begin),
                                   View_length(view),
                                   [](char *data, void *hint) {}, nullptr)
                     .ToLocalChecked();
  // TODO: nodeBuf->Set(Nan::New("dataOffset").ToLocalChecked(),
  //             Nan::New(static_cast<uint32_t>(view.offset())));
  return nodeBuf;
}

View Variant::Private::getView(v8::Local<v8::Object> obj) {
  using namespace v8;

  if (!node::Buffer::HasInstance(obj)) {
    return View();
  }

  size_t len = node::Buffer::Length(obj);
  char *data = new char[len];
  std::memcpy(data, node::Buffer::Data(obj), len);

  return View{data, data + len};
}

v8::Local<v8::Value> Variant::Private::getValue(const Variant &var) {
  switch (var.type()) {
  case TYPE_BOOL:
    return Nan::New(var.boolValue());
  case TYPE_INT64:
    return Nan::New(std::to_string(var.int64Value())).ToLocalChecked();
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
  case TYPE_VIEW:
    return getNodeBuffer(var.view());
  default:
    return Nan::Null();
  }
}

json11::Json Variant::Private::getJson(const Variant &var) {
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
  case Variant::TYPE_VIEW:
    json["type"] = "view";
    {
      std::stringstream stream;
      stream << std::hex << std::setfill('0') << std::setw(2);
      const auto &view = var.view();
      const uint8_t *data = reinterpret_cast<const uint8_t *>(view.begin);
      size_t length = View_length(view);
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

Variant Variant::Private::getVariant(v8::Local<v8::Value> var) {
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
    return getView(var.As<v8::Object>());
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
      map.emplace_back(key, value);
    }
    return map;
  }
  return Variant();
}

bool Variant_bool(const Variant *var) { return var->boolValue(); }
void Variant_setBool(Variant *var, bool value) { *var = Variant(value); }

int64_t Variant_int64(const Variant *var) { return var->int64Value(); }
void Variant_setInt64(Variant *var, int64_t value) { *var = Variant(value); }

uint64_t Variant_uint64(const Variant *var) { return var->uint64Value(); }
void Variant_setUint64(Variant *var, uint64_t value) { *var = Variant(value); }

double Variant_double(const Variant *var) { return var->doubleValue(); }
void Variant_setDouble(Variant *var, double value) { *var = Variant(value); }

const char *Variant_string(const Variant *var) {
  if (var->isString()) {
    return var->d.str->c_str();
  }
  return "";
}
void Variant_setString(Variant *var, const char *str) {
  *var = Variant(std::string(str));
}

View Variant_data(const Variant *var) {
  if (var->isView()) {
    return var->view();
  }
  return View{nullptr, nullptr};
}
void Variant_setData(Variant *var, View view) { *var = Variant(view); }
}
