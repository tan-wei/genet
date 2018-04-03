#include "dissector_thread_pool.hpp"
#include "dissector.hpp"
#include "dissector_thread.hpp"
#include "random_id.hpp"
#include "session_context.hpp"
#include "variant.hpp"
#include <array>

namespace plugkit {

class DissectorThreadPool::Private {
public:
  Private(const SessionContext *sctx);

public:
  const SessionContext *sctx;
  std::vector<std::unique_ptr<DissectorThread>> threads;
  std::vector<Dissector> dissectors;
  LoggerPtr logger = std::make_shared<StreamLogger>();
  FrameQueuePtr queue = std::make_shared<FrameQueue>();
  Callback callback;
  RootAllocator *allocator = nullptr;
  const std::string inspectorId = ":" + RandomID::generate<8>();
  InspectorCallback inspectorCallback;
  std::vector<std::string> inspectors;
};

DissectorThreadPool::Private::Private(const SessionContext *sctx)
    : sctx(sctx) {}

DissectorThreadPool::DissectorThreadPool(const SessionContext *sctx)
    : d(new Private(sctx)) {}

DissectorThreadPool::~DissectorThreadPool() {
  d->queue->close();
  for (const auto &thread : d->threads) {
    thread->join();
  }
}

void DissectorThreadPool::start() {
  if (!d->threads.empty())
    return;

  auto threadCallback = [this](Frame **begin, size_t size) {
    d->callback(begin, size);
  };

  int concurrency = std::stoi(d->sctx->config()["_.dissector.concurrency"]);
  if (concurrency == 0)
    concurrency = std::thread::hardware_concurrency();
  if (concurrency == 0)
    concurrency = 1;

  for (int i = 0; i < concurrency; ++i) {
    auto dissectorThread =
        new DissectorThread(d->sctx, d->queue, threadCallback);
    for (const auto &diss : d->dissectors) {
      dissectorThread->pushDissector(diss);
    }
    dissectorThread->setAllocator(d->allocator);

    const auto &inspector =
        "worker:dissector:" + std::to_string(i) + d->inspectorId;
    dissectorThread->setInspector(inspector,
                                  [this, inspector](const std::string &msg) {
                                    d->inspectorCallback(inspector, msg);
                                  });
    d->inspectors.push_back(inspector);

    d->threads.emplace_back(dissectorThread);
  }
  for (const auto &thread : d->threads) {
    thread->start();
  }
}

void DissectorThreadPool::setCallback(const Callback &callback) {
  d->callback = callback;
}

void DissectorThreadPool::setAllocator(RootAllocator *allocator) {
  d->allocator = allocator;
}

void DissectorThreadPool::registerDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void DissectorThreadPool::push(Frame **begin, size_t length) {
  d->queue->enqueue(begin, begin + length);
}

void DissectorThreadPool::sendInspectorMessage(const std::string &id,
                                               const std::string &msg) {
  for (size_t i = 0; i < d->inspectors.size(); ++i) {
    if (d->inspectors[i] == id) {
      d->threads[i]->sendInspectorMessage(msg);
      break;
    }
  }
}

void DissectorThreadPool::setInspectorCallback(
    const InspectorCallback &callback) {
  d->inspectorCallback = callback;
}

std::vector<std::string> DissectorThreadPool::inspectors() const {
  return d->inspectors;
}

} // namespace plugkit
