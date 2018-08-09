#include "filter.hpp"
#include "exports.hpp"
#include "frame.hpp"

namespace genet_node {

bool FilterIsolate::test(const Frame *frame) {
  v8::HandleScope handle_scope(isolate);
  if (testFunc.IsEmpty()) {
    return false;
  }
  auto func = v8::Local<v8::Function>::New(isolate, testFunc);
  v8::Local<v8::Value> args[] = {FrameWrapper::wrap(frame)};
  auto result = func->Call(func, 1, args);
  return !result.IsEmpty() && result->BooleanValue();
}

FilterIsolate::FilterIsolate(const std::string &data) {
  v8::HandleScope handle_scope(isolate);
  auto script = Nan::CompileScript(Nan::New(data.c_str()).ToLocalChecked());
  if (!script.IsEmpty()) {
    auto result = Nan::RunScript(script.ToLocalChecked());
    if (!result.IsEmpty()) {
      auto func = result.ToLocalChecked();
      if (func->IsFunction()) {
        testFunc.Reset(isolate, func.As<v8::Function>());
      }
    }
  }
}

FilterIsolate::~FilterIsolate() { testFunc.Reset(); }

Filter *FilterIsolate::createFilter(const char *data, size_t length) {
  Filter *filter = new Filter();
  filter->data = std::string(data, length);
  filter->new_worker = [](Filter *filter) -> FilterWorker * {
    FilterWorker *worker = new FilterWorker();
    worker->data = new FilterIsolate(filter->data);
    worker->test = [](FilterWorker *worker, const Frame *frame) -> uint8_t {
      return worker->data->test(frame);
    };
    worker->destroy = [](FilterWorker *worker) {
      delete worker->data;
      delete worker;
    };
    return worker;
  };
  filter->destroy = [](Filter *filter) { delete filter; };
  return filter;
}

} // namespace genet_node