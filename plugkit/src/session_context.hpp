#ifndef PLUGKIT_SESSION_CONTEXT_H
#define PLUGKIT_SESSION_CONTEXT_H

namespace plugkit {
struct SharedContextWrapper;

class SessionContext {
public:
  SessionContext();
  ~SessionContext();

private:
  SharedContextWrapper *shared;
};
} // namespace plugkit

#endif