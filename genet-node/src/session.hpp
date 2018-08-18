#ifndef GENET_NODE_SESSION_WRAPPER_H
#define GENET_NODE_SESSION_WRAPPER_H

#include "pointer.hpp"
#include <memory>
#include <mutex>
#include <nan.h>
#include <thread>
#include <uv.h>
#include <vector>

struct Session;
struct SessionProfile;

namespace genet_node {

class SessionProfileWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Local<v8::Object> exports);
  static Pointer<SessionProfile> unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_METHOD(setConfig);
  static NAN_METHOD(loadLibrary);
  static NAN_GETTER(concurrency);
  static NAN_SETTER(setConcurrency);

public:
  ~SessionProfileWrapper();

private:
  SessionProfileWrapper(const Pointer<SessionProfile> &session);
  SessionProfileWrapper(const SessionProfileWrapper &) = delete;
  SessionProfileWrapper &operator=(const SessionProfileWrapper &) = delete;

private:
  Pointer<SessionProfile> profile;
};

class SessionWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Local<v8::Object> exports);
  static NAN_METHOD(New);
  static NAN_GETTER(context);
  static NAN_METHOD(close);
  static NAN_METHOD(frames);
  static NAN_METHOD(filteredFrames);
  static NAN_METHOD(setFilter);
  static NAN_METHOD(createReader);
  static NAN_METHOD(createWriter);
  static NAN_METHOD(closeReader);
  static NAN_METHOD(closeWriter);
  static NAN_METHOD(getMetadata);
  static NAN_GETTER(callback);
  static NAN_SETTER(setCallback);
  static NAN_GETTER(length);

public:
  ~SessionWrapper();

private:
  SessionWrapper();
  void handleEvent(const char *event);
  void close();

  SessionWrapper(const SessionWrapper &) = delete;
  SessionWrapper &operator=(const SessionWrapper &) = delete;

private:
  struct Event {
    Session *session;
    std::mutex mutex;
    uv_async_t async;
    std::vector<char *> queue;
    v8::UniquePersistent<v8::Function> callback;
  };

private:
  Event *event;
};

} // namespace genet_node

#endif
