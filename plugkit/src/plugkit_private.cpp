#include "plugkit_private.hpp"
#include "extended_slot.hpp"
#include <map>
#include <v8.h>
#include <vector>

namespace plugkit {

class PlugKitPrivate::Private {
public:
};

PlugKitPrivate::PlugKitPrivate() : d(new Private()) {}

PlugKitPrivate &PlugKitPrivate::getInstance() {
  using namespace v8;
  Isolate *isolate = Isolate::GetCurrent();
  PlugKitPrivate *kit = ExtendedSlot::get<PlugKitPrivate>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_SINGLETON);
  if (!kit) {
    kit = new PlugKitPrivate();
    ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_SINGLETON, kit);
  }
  return *kit;
}
}
