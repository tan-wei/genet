#include "error.hpp"
#include "payload.hpp"
#include "reader.hpp"
#include "variant.hpp"

namespace plugkit {

ReaderWrapper::ReaderWrapper() { Reader_reset(&reader); }

void ReaderWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Reader").ToLocalChecked());
  SetPrototypeMethod(tpl, "slice", slice);
  SetPrototypeMethod(tpl, "sliceAll", sliceAll);
  SetPrototypeMethod(tpl, "getUint8", getUint8);
  SetPrototypeMethod(tpl, "getInt8", getInt8);
  SetPrototypeMethod(tpl, "getUint16", getUint16);
  SetPrototypeMethod(tpl, "getUint32", getUint32);
  SetPrototypeMethod(tpl, "getInt16", getInt16);
  SetPrototypeMethod(tpl, "getInt32", getInt32);
  SetPrototypeMethod(tpl, "getFloat32", getFloat32);
  SetPrototypeMethod(tpl, "getFloat64", getFloat64);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("data").ToLocalChecked(), data, setData);
  Nan::SetAccessor(otl, Nan::New("lastRange").ToLocalChecked(), lastRange,
                   setLastRange);
  Nan::SetAccessor(otl, Nan::New("lastError").ToLocalChecked(), lastError,
                   setLastError);

  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Reader").ToLocalChecked(), func);
}

NAN_METHOD(ReaderWrapper::New) {
  ReaderWrapper *obj = new ReaderWrapper();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(ReaderWrapper::data) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    const Slice &slice = reader->data;
    size_t sliceLen = Slice_length(slice);
    info.GetReturnValue().Set(v8::Uint8Array::New(
        v8::ArrayBuffer::New(v8::Isolate::GetCurrent(),
                             const_cast<char *>(slice.begin), sliceLen),
        0, sliceLen));
  }
}

NAN_SETTER(ReaderWrapper::setData) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (value->IsArrayBufferView()) {
      reader->data = Variant::getSlice(value.As<v8::ArrayBufferView>());
    }
  }
}

NAN_GETTER(ReaderWrapper::lastRange) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(static_cast<uint32_t>(reader->lastRange.begin)));
    array->Set(1, Nan::New(static_cast<uint32_t>(reader->lastRange.end)));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(ReaderWrapper::setLastRange) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (value->IsArray()) {
      auto array = value.As<v8::Array>();
      if (array->Length() >= 2) {
        reader->lastRange =
            Range{array->Get(0)->Uint32Value(), array->Get(1)->Uint32Value()};
      }
    }
  }
}

NAN_GETTER(ReaderWrapper::lastError) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(ErrorWrapper::wrap(reader->lastError));
  }
}

NAN_SETTER(ReaderWrapper::setLastError) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    reader->lastError = ErrorWrapper::unwrap(value.As<v8::Object>());
  }
}

NAN_METHOD(ReaderWrapper::slice) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (!info[0]->IsNumber()) {
      Nan::ThrowTypeError("First argument must be a number");
      return;
    }
    if (!info[1]->IsNumber()) {
      Nan::ThrowTypeError("Second argument must be a number");
      return;
    }
    const Slice &slice =
        Reader_slice(reader, info[0]->NumberValue(), info[1]->NumberValue());
    size_t sliceLen = Slice_length(slice);
    info.GetReturnValue().Set(v8::Uint8Array::New(
        v8::ArrayBuffer::New(v8::Isolate::GetCurrent(),
                             const_cast<char *>(slice.begin), sliceLen),
        0, sliceLen));
  }
}

NAN_METHOD(ReaderWrapper::sliceAll) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    size_t offset = 0;
    if (info.Length() >= 1) {
      if (info[0]->IsNumber()) {
        offset = info[0]->NumberValue();
      } else {
        Nan::ThrowTypeError("First argument must be a number");
        return;
      }
    }
    const Slice &slice = Reader_sliceAll(reader, offset);
    size_t sliceLen = Slice_length(slice);
    info.GetReturnValue().Set(v8::Uint8Array::New(
        v8::ArrayBuffer::New(v8::Isolate::GetCurrent(),
                             const_cast<char *>(slice.begin), sliceLen),
        0, sliceLen));
  }
}

NAN_METHOD(ReaderWrapper::getUint8) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(Reader_getUint8(reader));
  }
}

NAN_METHOD(ReaderWrapper::getInt8) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(Reader_getInt8(reader));
  }
}

NAN_METHOD(ReaderWrapper::getUint16) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (info.Length() >= 1 && !info[0]->IsBoolean()) {
      Nan::ThrowTypeError("First argument must be a boolean");
      return;
    }
    info.GetReturnValue().Set(
        Reader_getUint16(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getUint32) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (info.Length() >= 1 && !info[0]->IsBoolean()) {
      Nan::ThrowTypeError("First argument must be a boolean");
      return;
    }
    info.GetReturnValue().Set(
        Reader_getUint32(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getInt16) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (info.Length() >= 1 && !info[0]->IsBoolean()) {
      Nan::ThrowTypeError("First argument must be a boolean");
      return;
    }
    info.GetReturnValue().Set(Reader_getInt16(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getInt32) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (info.Length() >= 1 && !info[0]->IsBoolean()) {
      Nan::ThrowTypeError("First argument must be a boolean");
      return;
    }
    info.GetReturnValue().Set(Reader_getInt32(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getFloat32) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (info.Length() >= 1 && !info[0]->IsBoolean()) {
      Nan::ThrowTypeError("First argument must be a boolean");
      return;
    }
    info.GetReturnValue().Set(
        Reader_getFloat32(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getFloat64) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    if (info.Length() >= 1 && !info[0]->IsBoolean()) {
      Nan::ThrowTypeError("First argument must be a boolean");
      return;
    }
    info.GetReturnValue().Set(
        Reader_getFloat64(reader, info[0]->BooleanValue()));
  }
}
} // namespace plugkit
