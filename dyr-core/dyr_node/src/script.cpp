#include "script.hpp"
#include "module.hpp"

namespace dyr_node {

class ArrayBufferAllocator final : public v8::ArrayBuffer::Allocator {
 public:
  ArrayBufferAllocator() {}
  ~ArrayBufferAllocator() override {}
  void* Allocate(size_t size) override { return calloc(1, size); }
  void* AllocateUninitialized(size_t size) override { return malloc(size); }
  void Free(void* data, size_t) override { free(data); }
};

namespace {
struct Context {
  int refCount = 0;
  std::unique_ptr<ArrayBufferAllocator> allocator;
  std::unique_ptr<v8::Locker> locker;
  v8::UniquePersistent<v8::Context> context;
};
thread_local Context global = Context();
}  // namespace

Script::Script() : isolate(nullptr) {
  if (global.refCount == 0) {
    global.allocator.reset(new ArrayBufferAllocator());
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = global.allocator.get();
    isolate = v8::Isolate::New(create_params);
    isolate->Enter();
    if (v8::Locker::IsActive()) {
      global.locker.reset(new v8::Locker(isolate));
    }
    v8::HandleScope handle_scope(isolate);
    auto ctx = v8::Context::New(isolate);
    global.context.Reset(isolate, ctx);
    ctx->Enter();
    Module::init(ctx->Global());
  } else {
    isolate = v8::Isolate::GetCurrent();
  }
  global.refCount += 1;
}

Script::~Script() {
  global.refCount -= 1;
  if (global.refCount == 0) {
    {
      v8::HandleScope handle_scope(isolate);
      Module::destroy();
      v8::Local<v8::Context>::New(isolate, global.context)->Exit();
      global.context.Reset();
      global.locker.reset();
    }
    isolate->Exit();
    isolate->Dispose();
    global = Context();
  }
}

}  // namespace dyr_node