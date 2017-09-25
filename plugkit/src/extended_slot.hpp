#ifndef PLUGKIT_EXTENDED_SLOT_HPP
#define PLUGKIT_EXTENDED_SLOT_HPP

#include <v8.h>

namespace plugkit {
namespace ExtendedSlot {

constexpr int isolateSlot = 2;

enum SubSlot {
  SLOT_ELECTRON = 0,
  SLOT_PLUGKIT_SINGLETON = 1,
  SLOT_PLUGKIT_MODULE = 2
};

inline void init(v8::Isolate *isolate) {
  using namespace v8;
  if (!isolate->GetData(isolateSlot)) {
    isolate->SetData(isolateSlot, new void *[8]);
  }
}

inline void destroy(v8::Isolate *isolate) {
  delete[] static_cast<void **>(isolate->GetData(isolateSlot));
}

inline void set(v8::Isolate *isolate, SubSlot slot, void *ptr) {
  if (auto slots = static_cast<void **>(isolate->GetData(isolateSlot))) {
    slots[slot] = ptr;
  }
}

template <class T> T *get(v8::Isolate *isolate, SubSlot slot) {
  if (auto slots = static_cast<void **>(isolate->GetData(isolateSlot))) {
    return static_cast<T *>(slots[slot]);
  } else {
    return nullptr;
  }
}
} // namespace ExtendedSlot
} // namespace plugkit

#endif
