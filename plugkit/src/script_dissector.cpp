#include "script_dissector.hpp"
#include "private/logger.hpp"
#include "private/variant.hpp"
#include "session_context.hpp"
#include "wrapper/layer.hpp"
#include <nan.h>

namespace plugkit {

class ScriptDissector::Worker::Private {
public:
  v8::UniquePersistent<v8::Object> workerObj;
  v8::UniquePersistent<v8::Function> workerFunc;
  SessionContext ctx;
};

ScriptDissector::Worker::Worker(
    const SessionContext &ctx,
    const v8::UniquePersistent<v8::Object> &workerObj)
    : d(new Private()) {
  d->ctx = ctx;
  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  if (workerObj.IsEmpty())
    return;
  d->workerObj.Reset(isolate, workerObj);
  auto analyzeValue = Local<Object>::New(isolate, workerObj)
                          ->Get(Nan::New("analyze").ToLocalChecked());
  if (!analyzeValue->IsFunction())
    return;
  d->workerFunc.Reset(isolate, analyzeValue.As<Function>());
}

ScriptDissector::Worker::~Worker() {
  d->workerObj.Reset();
  d->workerFunc.Reset();
}

LayerPtr ScriptDissector::Worker::analyze(const LayerConstPtr &layer) {
  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  v8::TryCatch tryCatch(isolate);
  auto func = Local<Function>::New(isolate, d->workerFunc);
  auto obj = Local<Object>::New(isolate, d->workerObj);
  if (func.IsEmpty() || obj.IsEmpty()) {
    return LayerPtr();
  }

  Local<Value> args[1] = {LayerWrapper::wrap(layer)};
  auto result = func->Call(obj, 1, args);
  if (tryCatch.HasCaught()) {
    auto msg = Logger::Private::fromV8Message(
        tryCatch.Message(), Logger::LEVEL_ERROR, "script_dissector");
    d->ctx.logger->log(std::move(msg));
    return LayerPtr();
  }
  if (result->IsObject()) {
    if (const LayerPtr &child = LayerWrapper::unwrap(result.As<Object>())) {
      return child;
    }
  }
  return LayerPtr();
}

class ScriptDissector::Private {
public:
  void init(const SessionContext &ctx, const std::string &script,
            const std::string &path, v8::TryCatch *tryCatch);

public:
  std::vector<strns> namespaces;
  v8::UniquePersistent<v8::Object> workerObj;
  SessionContext ctx;
};

void ScriptDissector::Private::init(const SessionContext &ctx,
                                    const std::string &script,
                                    const std::string &path,
                                    v8::TryCatch *tryCatch) {

  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();

  auto source =
      Nan::New("(function(module){\n" + script + "\n})").ToLocalChecked();
  auto scriptObj = v8::Script::Compile(context, source);
  if (tryCatch->HasCaught())
    return;

  auto result = scriptObj.ToLocalChecked()->Run(context);
  if (tryCatch->HasCaught())
    return;

  auto funcValue = result.ToLocalChecked();
  if (!funcValue->IsFunction()) {
    return;
  }

  auto func = funcValue.As<Function>();
  auto moduleObj = Nan::New<Object>();
  Local<Value> moduleValue = moduleObj;
  func->Call(moduleObj, 1, &moduleValue);
  if (tryCatch->HasCaught())
    return;

  auto exportsValue = moduleObj->Get(Nan::New("exports").ToLocalChecked());
  if (tryCatch->HasCaught())
    return;

  if (!exportsValue->IsFunction()) {
    return;
  }

  auto ctor = exportsValue.As<Function>();
  auto nsValue = ctor->Get(Nan::New("namespaces").ToLocalChecked());
  if (tryCatch->HasCaught())
    return;

  if (!nsValue->IsArray()) {
    return;
  }

  auto nsArray = nsValue.As<Array>();
  for (uint32_t i = 0; i < nsArray->Length(); ++i) {
    auto itemValue = nsArray->Get(i);
    if (itemValue->IsString()) {
      namespaces.emplace_back(*Nan::Utf8String(itemValue));
    }
  }
  auto sess = Nan::New<v8::Object>();
  sess->Set(Nan::New("options").ToLocalChecked(),
            Variant::Private::getValue(ctx.options));
  v8::Local<v8::Value> args[1] = {sess};
  auto worker = ctor->NewInstance(context, 1, args);

  if (tryCatch->HasCaught())
    return;

  workerObj.Reset(isolate, worker.ToLocalChecked());
}

ScriptDissector::ScriptDissector(const SessionContext &ctx,
                                 const std::string &script,
                                 const std::string &path)
    : d(new Private()) {

  d->ctx = ctx;

  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  v8::TryCatch tryCatch(isolate);
  d->init(ctx, script, path, &tryCatch);
  if (tryCatch.HasCaught()) {
    auto msg = Logger::Private::fromV8Message(
        tryCatch.Message(), Logger::LEVEL_ERROR, "script_dissector");
    ctx.logger->log(std::move(msg));
    return;
  }
}

ScriptDissector::~ScriptDissector() { d->workerObj.Reset(); }

Dissector::WorkerPtr ScriptDissector::createWorker() {
  if (d->workerObj.IsEmpty()) {
    return Dissector::WorkerPtr();
  }
  return Dissector::WorkerPtr(
      new ScriptDissector::Worker(d->ctx, d->workerObj));
}

std::vector<strns> ScriptDissector::namespaces() const { return d->namespaces; }

class ScriptDissectorFactory::Private {
public:
  std::string script;
  std::string path;
};

ScriptDissectorFactory::ScriptDissectorFactory(const std::string &script,
                                               const std::string &path)
    : d(new Private()) {
  d->script = script;
  d->path = path;
}

ScriptDissectorFactory::~ScriptDissectorFactory() {}

DissectorPtr
ScriptDissectorFactory::create(const SessionContext &context) const {
  return DissectorPtr(new ScriptDissector(context, d->script, d->path));
}
}
