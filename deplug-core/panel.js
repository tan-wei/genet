const registry = {}
export default class Panel {
  static mount (slot, component, options = {}) {
    if (slot in registry) {
      registry[slot](component, options)
    }
  }

  static registerSlot (slot, handler) {
    registry[slot] = handler
  }
}
