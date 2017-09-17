#ifndef PLUGKIT_READER_WRAPPER_H
#define PLUGKIT_READER_WRAPPER_H

#include <memory>
#include <nan.h>
#include "reader.h"

namespace plugkit {

class ReaderWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static NAN_METHOD(New);
  static NAN_GETTER(data);
  static NAN_SETTER(setData);
  static NAN_GETTER(lastRange);
  static NAN_SETTER(setLastRange);
  static NAN_METHOD(slice);
  static NAN_METHOD(sliceAll);
  static NAN_METHOD(getUint8);
  static NAN_METHOD(getInt8);
  static NAN_METHOD(getUint16);
  static NAN_METHOD(getUint32);
  static NAN_METHOD(getInt16);
  static NAN_METHOD(getInt32);
  static NAN_METHOD(getFloat32);
  static NAN_METHOD(getFloat64);

private:
  ReaderWrapper();
  ReaderWrapper(const ReaderWrapper &) = delete;
  ReaderWrapper &operator=(const ReaderWrapper &) = delete;

private:
  Reader reader;
};
}

#endif
