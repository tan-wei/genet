#ifndef PLUGKIT_SESSION_CONTEXT_H
#define PLUGKIT_SESSION_CONTEXT_H

namespace plugkit {
struct SharedContext;

class SessionContext {
public:
  SessionContext();
  ~SessionContext();

private:
  SharedContext *shared;
};
} // namespace plugkit

#endif