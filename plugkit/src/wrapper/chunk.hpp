#ifndef PLUGKIT_CHUNK_WRAPPER_H
#define PLUGKIT_CHUNK_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class Chunk;

class ChunkWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Chunk *chunk);
  static const Chunk *unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(streamNs);
  static NAN_SETTER(setStreamNs);
  static NAN_GETTER(streamId);
  static NAN_SETTER(setStreamId);
  static NAN_GETTER(payload);
  static NAN_SETTER(setPayload);
  static NAN_GETTER(properties);
  static NAN_METHOD(propertyFromId);
  static NAN_METHOD(addProperty);
  static NAN_METHOD(toJSON);
  static NAN_GETTER(layer);

private:
  ChunkWrapper(Chunk *chunk);
  ChunkWrapper(const Chunk *chunk);
  ChunkWrapper(const ChunkWrapper &) = delete;
  ChunkWrapper &operator=(const ChunkWrapper &) = delete;

private:
  std::shared_ptr<Chunk> chunk;
  const Chunk *constChunk;
};
}

#endif
