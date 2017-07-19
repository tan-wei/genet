#ifndef PLUGKIT_CHUNK_WRAPPER_H
#define PLUGKIT_CHUNK_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class Chunk;

class ChunkWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const std::shared_ptr<Chunk> &attr);
  static v8::Local<v8::Object> wrap(const std::weak_ptr<const Chunk> &attr);
  static std::shared_ptr<Chunk> unwrap(v8::Local<v8::Object> obj);
  static std::shared_ptr<const Chunk> unwrapConst(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(layer);
  static NAN_SETTER(setLayer);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);

private:
  ChunkWrapper(const std::shared_ptr<Chunk> &chunk);
  ChunkWrapper(const std::weak_ptr<Chunk> &chunk);
  ChunkWrapper(const ChunkWrapper &) = delete;
  ChunkWrapper &operator=(const ChunkWrapper &) = delete;

private:
  std::shared_ptr<Chunk> chunk;
  std::weak_ptr<const Chunk> weakChunk;
};
}

#endif
