const globalRegistry = {}
export default class Tab {
  static register (tab) {
    globalRegistry[tab.name] = tab
  }

  static get registry () {
    return globalRegistry
  }

  static get (name) {
    return globalRegistry[name]
  }
}
