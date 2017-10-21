#include "filter.hpp"
#include "attr.hpp"
#include "frame.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "wrapper/attr.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/layer.hpp"
#include <functional>
#include <json11.hpp>

namespace plugkit {

using namespace v8;

class Filter::Private {
public:
  v8::UniquePersistent<v8::Function> func;
};

Filter::Filter(const std::string &body) : d(new Private()) {
  auto script = Nan::CompileScript(Nan::New(body).ToLocalChecked());
  if (!script.IsEmpty()) {
    auto result = Nan::RunScript(script.ToLocalChecked());
    if (!result.IsEmpty()) {
      auto func = result.ToLocalChecked();
      if (func->IsFunction()) {
        d->func.Reset(v8::Isolate::GetCurrent(), func.As<v8::Function>());
      }
    }
  }
}

Filter::~Filter() {}

void Filter::test(char *results, const FrameView **begin, size_t size) const {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto global = isolate->GetCurrentContext()->Global();
  auto func = v8::Local<v8::Function>::New(isolate, d->func);
  if (!func.IsEmpty()) {
    for (size_t i = 0; i < size; ++i) {
      v8::Local<v8::Value> args[1] = {FrameWrapper::wrap(begin[i])};
      auto value = func->Call(global, 1, args);
      if (!value.IsEmpty()) {
        results[i] = value->BooleanValue();
      }
    }
  }
}
} // namespace plugkit
