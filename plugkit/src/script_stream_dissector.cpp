#include "script_stream_dissector.hpp"
#include "private/logger.hpp"
#include "private/variant.hpp"
#include "session_context.hpp"
#include "wrapper/chunk.hpp"
#include "wrapper/layer.hpp"
#include <nan.h>

namespace plugkit {

namespace {
std::regex constantRegex(const std::string &str) {
  static const std::regex meta("[-\\/\\\\^$*+?.()|[\\]{}]");
  return std::regex(std::regex_replace(str, meta, "\\$&"));
}
}

class ScriptStreamDissector::Worker::Private {
public:
  v8::UniquePersistent<v8::Object> workerObj;
  v8::UniquePersistent<v8::Function> analyzeFunc;
  v8::UniquePersistent<v8::Function> expiredFunc;
  SessionContext ctx;
};

ScriptStreamDissector::Worker::Worker(
    const SessionContext &ctx,
    const v8::UniquePersistent<v8::Object> &workerObj)
    : d(new Private()) {
  d->ctx = ctx;

  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  d->workerObj.Reset(isolate, workerObj);
  v8::TryCatch tryCatch(isolate);

  auto worker = Local<Object>::New(isolate, workerObj);
  if (worker.IsEmpty()) {
    return;
  }

  auto analyzeValue = worker->Get(Nan::New("analyze").ToLocalChecked());
  if (tryCatch.HasCaught()) {
    auto msg = Logger::Private::fromV8Message(
        tryCatch.Message(), Logger::LEVEL_ERROR, "script_stream_dissector");
    d->ctx.logger->log(std::move(msg));
    return;
  }

  if (!analyzeValue->IsFunction()) {
    return;
  }
  d->analyzeFunc.Reset(isolate, analyzeValue.As<Function>());
  auto expiredValue = worker->Get(Nan::New("expired").ToLocalChecked());
  if (tryCatch.HasCaught()) {
    auto msg = Logger::Private::fromV8Message(
        tryCatch.Message(), Logger::LEVEL_ERROR, "script_stream_dissector");
    d->ctx.logger->log(std::move(msg));
    return;
  }

  if (expiredValue->IsFunction()) {
    d->expiredFunc.Reset(isolate, expiredValue.As<Function>());
  }
}

ScriptStreamDissector::Worker::~Worker() {
  d->workerObj.Reset();
  d->analyzeFunc.Reset();
  d->expiredFunc.Reset();
}

LayerPtr ScriptStreamDissector::Worker::analyze(const ChunkConstPtr &chunk) {
  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  v8::TryCatch tryCatch(isolate);
  auto func = Local<Function>::New(isolate, d->analyzeFunc);
  auto obj = Local<Object>::New(isolate, d->workerObj);
  if (func.IsEmpty() || obj.IsEmpty()) {
    return LayerPtr();
  }

  Local<Value> args[1] = {ChunkWrapper::wrap(chunk)};
  auto result = func->Call(obj, 1, args);

  if (tryCatch.HasCaught()) {
    auto msg = Logger::Private::fromV8Message(
        tryCatch.Message(), Logger::LEVEL_ERROR, "script_stream_dissector");
    d->ctx.logger->log(std::move(msg));
    return LayerPtr();
  }

  if (result->IsObject()) {
    if (const LayerPtr &layer = LayerWrapper::unwrap(result.As<Object>())) {
      return layer;
    }
  }
  return LayerPtr();
}

bool ScriptStreamDissector::Worker::expired(
    const Timestamp &lastUpdated) const {
  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  v8::TryCatch tryCatch(isolate);
  auto func = Local<Function>::New(isolate, d->analyzeFunc);
  if (!func.IsEmpty()) {
    auto obj = Local<Object>::New(isolate, d->workerObj);
    Local<Value> args[1] = {Variant::Private::getValue(Variant(lastUpdated))};
    auto result = func->Call(obj, 1, args);

    if (tryCatch.HasCaught()) {
      auto msg = Logger::Private::fromV8Message(
          tryCatch.Message(), Logger::LEVEL_ERROR, "script_stream_dissector");
      d->ctx.logger->log(std::move(msg));
      return true;
    }

    return result->BooleanValue();
  } else {
    return StreamDissector::Worker::expired(lastUpdated);
  }
}

class ScriptStreamDissector::Private {
public:
  void init(const SessionContext &ctx, const std::string &script,
            const std::string &path, v8::TryCatch *tryCatch);

public:
  std::vector<std::regex> namespaces;
  v8::UniquePersistent<v8::Object> workerObj;
  SessionContext ctx;
};

void ScriptStreamDissector::Private::init(const SessionContext &ctx,
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
    if (itemValue->IsRegExp()) {
      namespaces.emplace_back(
          *Nan::Utf8String(itemValue.As<v8::RegExp>()->GetSource()));
    } else if (itemValue->IsString()) {
      namespaces.push_back(constantRegex(*Nan::Utf8String(itemValue)));
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

ScriptStreamDissector::ScriptStreamDissector(const SessionContext &ctx,
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
        tryCatch.Message(), Logger::LEVEL_ERROR, "script_stream_dissector");
    ctx.logger->log(std::move(msg));
    return;
  }
}

ScriptStreamDissector::~ScriptStreamDissector() { d->workerObj.Reset(); }

StreamDissector::WorkerPtr ScriptStreamDissector::createWorker() {
  if (d->workerObj.IsEmpty()) {
    return StreamDissector::WorkerPtr();
  }
  return StreamDissector::WorkerPtr(
      new ScriptStreamDissector::Worker(d->ctx, d->workerObj));
}

std::vector<std::regex> ScriptStreamDissector::namespaces() const {
  return d->namespaces;
}

class ScriptStreamDissectorFactory::Private {
public:
  std::string script;
  std::string path;
};

ScriptStreamDissectorFactory::ScriptStreamDissectorFactory(
    const std::string &script, const std::string &path)
    : d(new Private()) {
  d->script = script;
  d->path = path;
}

ScriptStreamDissectorFactory::~ScriptStreamDissectorFactory() {}

StreamDissectorPtr
ScriptStreamDissectorFactory::create(const SessionContext &context) const {
  return StreamDissectorPtr(
      new ScriptStreamDissector(context, d->script, d->path));
}
}
