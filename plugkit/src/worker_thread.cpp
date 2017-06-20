#include "worker_thread.hpp"
#include "extended_slot.hpp"
#include "plugkit_module.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <nan.h>
#include <node.h>
#include <sstream>
#include <string>
#include <thread>
#include <uv.h>
#include <v8.h>

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

WorkerThread::WorkerThread(bool node) : nodeIntegration(node) {}

WorkerThread::~WorkerThread() { join(); }

void WorkerThread::join() {
  if (thread.joinable())
    thread.join();
}

void WorkerThread::start() {
  if (thread.joinable())
    return;

  std::string fileName;
  {
    char tmpPath[2048];
    size_t tmpPathSize = sizeof(tmpPath);
    uv_os_tmpdir(tmpPath, &tmpPathSize);
    std::stringstream name;
    name << tmpPath << "/__plugkit_node_init_.js";
    fileName = name.str();
    std::ofstream ofs;
    ofs.open(fileName, std::ofstream::out);
    ofs << R"(
          global.require = function(id) {
            if (id === "plugkit") {
              return global._plugkit
            } else {
              return require(id)
            }
          };
        )";
    logger->log(Logger::LEVEL_DEBUG, std::string("init.js: ") + fileName,
                "worker_thread");
  }

  thread = std::thread([this, fileName]() {
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

      uv_loop_s uvloop;
      uv_loop_init(&uvloop);

      auto idata = node::CreateIsolateData(isolate, &uvloop);
      const char *args[2] = {"node", fileName.c_str()};

      static std::mutex nodeLock;
      node::Environment *env = nullptr;
      if (nodeIntegration) {
        std::lock_guard<std::mutex> lock(nodeLock);
        env = node::CreateEnvironment(idata, context, 2, args, 0, nullptr);
        node::LoadEnvironment(env);
        isolate->SetData(1, this);
      }
      ExtendedSlot::init(isolate);
      {
        v8::Local<v8::Object> exports = Nan::New<v8::Object>();
        std::unique_ptr<PlugkitModule> mod(
            new PlugkitModule(isolate, exports, false));
        context->Global()->Set(Nan::New("_plugkit").ToLocalChecked(), exports);
        enter();
        while (loop()) {
          uv_run(&uvloop, UV_RUN_NOWAIT);
        }
      }
      exit();
      if (nodeIntegration) {
        std::lock_guard<std::mutex> lock(nodeLock);
        node::FreeEnvironment(env);
        node::FreeIsolateData(idata);
      }
      ExtendedSlot::destroy(isolate);
    }
    isolate->Dispose();
    delete create_params.array_buffer_allocator;
    logger->log(Logger::LEVEL_DEBUG, "exit", "worker_thread");
  });
}

void WorkerThread::setLogger(const LoggerPtr &logger) { this->logger = logger; }
}
