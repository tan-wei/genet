const globalRegistry = {}
export default class Tab {
  static registerTemplate (tab) {
    globalRegistry[tab.name] = tab
  }

  static get registry () {
    return globalRegistry
  }

  static getTemplate (name) {
    return globalRegistry[name]
  }
}
