#include "script_dissector.hpp"
#include "wrapper/context.hpp"
#include "wrapper/layer.hpp"
#include <nan.h>

namespace plugkit {

namespace {
struct WorkerHolder {
  v8::UniquePersistent<v8::Object> worker;
  v8::UniquePersistent<v8::Function> analyze;
};
} // namespace

ScriptDissector::ScriptDissector(const v8::Local<v8::Function> &ctor)
    : func(v8::Isolate::GetCurrent(), ctor) {}

Dissector ScriptDissector::create(char *script) {
  Dissector dissector;
  std::memset(&dissector, 0, sizeof(dissector));
  dissector.data = script;
  dissector.initialize = [](Context *ctx, Dissector *diss) {
    const char *str = static_cast<const char *>(diss->data);
    diss->data = nullptr;

    auto script = Nan::CompileScript(Nan::New(str).ToLocalChecked());
    if (script.IsEmpty())
      return;
    auto result = Nan::RunScript(script.ToLocalChecked());
    if (result.IsEmpty())
      return;
    auto func = result.ToLocalChecked();
    if (!func->IsFunction())
      return;
    auto module = Nan::New<v8::Object>();
    v8::Local<v8::Value> args[1] = {module};
    func.As<v8::Function>()->Call(module, 1, args);
    auto exports = module->Get(Nan::New("exports").ToLocalChecked());
    if (!exports->IsFunction())
      return;
    auto ctor = exports.As<v8::Function>();
    auto hints = ctor->Get(Nan::New("layerHints").ToLocalChecked());
    if (hints->IsArray()) {
      auto layerHints = hints.As<v8::Array>();
      const uint32_t size =
          sizeof(diss->layerHints) / sizeof(diss->layerHints[0]);
      for (uint32_t i = 0; i < size && i < layerHints->Length(); ++i) {
        auto item = layerHints->Get(i);
        Token token = Token_null();
        if (item->IsNumber()) {
          token = item->NumberValue();
        } else if (item->IsString()) {
          token = Token_get(*Nan::Utf8String(item));
        }
        diss->layerHints[i] = token;
      }
    }
    auto scriptDissector = new ScriptDissector(ctor);
    diss->data = scriptDissector;
  };
  dissector.terminate = [](Context *ctx, Dissector *diss) {
    delete static_cast<ScriptDissector *>(diss->data);
  };
  dissector.createWorker = [](Context *ctx, const Dissector *diss) {
    ScriptDissector *scriptDissector =
        static_cast<ScriptDissector *>(diss->data);
    auto ctor = v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(),
                                             scriptDissector->func);
    auto worker = Nan::NewInstance(ctor).ToLocalChecked();
    auto analyze = worker->Get(Nan::New("analyze").ToLocalChecked());
    WorkerHolder *holder = nullptr;
    if (analyze->IsFunction()) {
      holder = new WorkerHolder();
      holder->worker.Reset(v8::Isolate::GetCurrent(), worker);
      holder->analyze.Reset(v8::Isolate::GetCurrent(),
                            analyze.As<v8::Function>());
    }
    return Worker{holder};
  };
  dissector.destroyWorker = [](Context *ctx, const Dissector *diss,
                               Worker worker) {
    auto holder = static_cast<WorkerHolder *>(worker.data);
    delete holder;
  };
  dissector.analyze = [](Context *ctx, const Dissector *diss, Worker worker,
                         Layer *layer) {
    auto holder = static_cast<WorkerHolder *>(worker.data);
    if (holder) {
      auto obj =
          v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), holder->worker);
      auto analyze = v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(),
                                                  holder->analyze);
      v8::Local<v8::Value> args[] = {ContextWrapper::wrap(ctx),
                                     LayerWrapper::wrap(layer)};
      analyze->Call(obj, 2, args);
    }
  };
  return dissector;
}
} // namespace plugkit
