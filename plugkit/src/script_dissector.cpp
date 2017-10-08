#include "script_dissector.hpp"
#include <nan.h>

namespace plugkit {

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
  dissector.analyze = [](Context *ctx, const Dissector *diss, Worker data,
                         Layer *layer) {};
  return dissector;
}
} // namespace plugkit
