#include "worker_thread.hpp"
#include "extended_slot.hpp"
#include "logger.hpp"
#include "plugkit_module.hpp"
#include "session_context.hpp"
#include "swap_queue.hpp"
#include "wrapper/logger.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <nan.h>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <uv.h>
#include <v8-inspector.h>

namespace plugkit {

class WorkerThread::ArrayBufferAllocator final
    : public v8::ArrayBuffer::Allocator {
public:
  ArrayBufferAllocator() {}
  ~ArrayBufferAllocator() override {}
  void *Allocate(size_t size) override { return calloc(1, size); }
  void *AllocateUninitialized(size_t size) override { return malloc(size); }
  void Free(void *data, size_t) override { free(data); }
};

class WorkerThread::InspectorClient : public v8_inspector::V8InspectorClient {
public:
  void runMessageLoopOnPause(int contextGroupId) override { mPaused = true; }

  void quitMessageLoopOnPause() override { mPaused = false; }

  bool paused() const { return mPaused; }

private:
  bool mPaused = false;
};

namespace {
std::string onebyte(const uint16_t *data, size_t length) {
  auto string = v8::String::NewFromTwoByte(v8::Isolate::GetCurrent(), data,
                                           v8::NewStringType::kNormal, length)
                    .ToLocalChecked();
  std::string utf8;
  utf8.resize(string->Utf8Length());
  string->WriteUtf8(&utf8[0]);
  return utf8;
}
} // namespace

class WorkerThread::InspectorChannel
    : public v8_inspector::V8Inspector::Channel {
public:
  InspectorChannel(const InspectorCallback &callback) : callback(callback) {}
  void
  sendResponse(int callId,
               std::unique_ptr<v8_inspector::StringBuffer> message) override {
    const auto &view = message->string();

    auto str = onebyte(view.characters16(), view.length());
    if (callback)
      callback(str);
  }
  void sendNotification(
      std::unique_ptr<v8_inspector::StringBuffer> message) override {
    const auto &view = message->string();

    auto str = onebyte(view.characters16(), view.length());
    if (callback)
      callback(str);
  }
  void flushProtocolNotifications() override {}

private:
  const InspectorCallback &callback;
};

class WorkerThread::Private {
public:
  Private(const SessionContext *sctx);

public:
  const SessionContext *sctx;
  std::string inspectorId;
  InspectorCallback inspectorCallback;
  SwapQueue<std::string> inspectorQueue;
  bool inspectorActivated = false;
};

WorkerThread::Private::Private(const SessionContext *sctx) : sctx(sctx) {}

WorkerThread::WorkerThread(const SessionContext *sctx) : d(new Private(sctx)) {}

WorkerThread::~WorkerThread() {}

void WorkerThread::join() {
  if (thread.joinable())
    thread.join();
}

void WorkerThread::start() {
  if (thread.joinable())
    return;

  thread = std::thread([this]() {
    auto logger = d->sctx->logger();

    logger->log(Logger::LEVEL_DEBUG, "start", "worker_thread");

    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = new ArrayBufferAllocator();
    v8::Isolate *isolate = v8::Isolate::New(create_params);

    {
      std::unique_ptr<v8::Locker> locker;
      v8::Isolate::Scope isolate_scope(isolate);
      if (v8::Locker::IsActive()) {
        locker.reset(new v8::Locker(isolate));
      }
      v8::HandleScope handle_scope(isolate);

      v8::Local<v8::Context> context = v8::Context::New(isolate);
      v8::Context::Scope context_scope(context);

      std::unique_ptr<WorkerThread::InspectorClient> inspectorClient;
      std::unique_ptr<v8_inspector::V8Inspector> inspector;
      std::unique_ptr<v8_inspector::V8Inspector::Channel> channel;
      std::unique_ptr<v8_inspector::V8InspectorSession> session;

      if (!d->inspectorId.empty()) {
        inspectorClient.reset(new InspectorClient());
        inspector =
            v8_inspector::V8Inspector::create(isolate, inspectorClient.get());
        channel.reset(new InspectorChannel(d->inspectorCallback));
        session =
            inspector->connect(1, channel.get(), v8_inspector::StringView());
        inspector->contextCreated(v8_inspector::V8ContextInfo(
            context, 1,
            v8_inspector::StringView(
                reinterpret_cast<const uint8_t *>(d->inspectorId.c_str()),
                d->inspectorId.size())));
      }

      uv_loop_s uvloop;
      uv_loop_init(&uvloop);

      ExtendedSlot::init(isolate);
      {
        v8::Local<v8::Object> exports = Nan::New<v8::Object>();
        std::unique_ptr<PlugkitModule> mod(
            new PlugkitModule(isolate, exports, false));
        auto global = context->Global();
        global->Set(Nan::New("_plugkit").ToLocalChecked(), exports);
        global->Set(Nan::New("console").ToLocalChecked(),
                    LoggerWrapper::wrap(logger));
        global->Set(
            Nan::New("require").ToLocalChecked(),
            v8::FunctionTemplate::New(
                isolate,
                [](v8::FunctionCallbackInfo<v8::Value> const &info) {
                  if (info[0]->IsString() &&
                      std::strcmp("plugkit", *Nan::Utf8String(info[0])) == 0) {
                    info.GetReturnValue().Set(info.Data());
                  }
                },
                exports)
                ->GetFunction());
        enter();
        while (loop()) {
          if (session) {
            do {
              for (const auto &msg : d->inspectorQueue.fetch()) {
                d->inspectorActivated = true;
                session->dispatchProtocolMessage(v8_inspector::StringView(
                    reinterpret_cast<const uint8_t *>(msg.c_str()),
                    msg.size()));
              }
            } while (inspectorClient->paused());
          }
          uv_run(&uvloop, UV_RUN_NOWAIT);
        }
      }
      exit();
      ExtendedSlot::destroy(isolate);
    }
    isolate->Dispose();
    delete create_params.array_buffer_allocator;
    logger->log(Logger::LEVEL_DEBUG, "exit", "worker_thread");
  });
}

bool WorkerThread::inspectorActivated() const { return d->inspectorActivated; }

void WorkerThread::sendInspectorMessage(const std::string &msg) {
  d->inspectorQueue.push(msg);
}

void WorkerThread::setInspector(const std::string &id,
                                const InspectorCallback &callback) {
  d->inspectorId = id;
  d->inspectorCallback = callback;
}
} // namespace plugkit
