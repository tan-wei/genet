#include "session_context.hpp"
#include "allocator.hpp"
#include "config_map.hpp"
#include "stream_logger.hpp"

namespace plugkit {
extern "C" {
SharedContextWrapper *plugkit_in_create_shared_ctx();
void plugkit_in_destroy_shared_ctx(SharedContextWrapper *ctx);
}

class SessionContext::Private {
public:
  Private();
  ~Private();

public:
  SharedContextWrapper *context;
  RootAllocator allocator;
  ConfigMap config;
  LoggerPtr logger = std::make_shared<StreamLogger>();
};

SessionContext::Private::Private() : context(plugkit_in_create_shared_ctx()) {}

SessionContext::Private::~Private() { plugkit_in_destroy_shared_ctx(context); }

SessionContext::SessionContext() : d(new Private()) {}

SessionContext::~SessionContext() {}

SharedContextWrapper *SessionContext::context() const { return d->context; }

RootAllocator *SessionContext::allocator() const { return &d->allocator; }

const LoggerPtr &SessionContext::logger() const { return d->logger; }

void SessionContext::setLogger(const LoggerPtr &logger) { d->logger = logger; }

const ConfigMap &SessionContext::config() const { return d->config; }

void SessionContext::setConfig(const ConfigMap &config) { d->config = config; }

} // namespace plugkit