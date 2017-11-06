const fields = Symbol('fields')
export default class PackageManager {
  constructor (config) {
    this[fields] = {
      list: [],
      enabledComponents: new Set(),
    }
  }

  enableComponent (type) {
    this[fields].enabledComponents.add(type)
  }

  get list () {
    return this[fields].list
  }
}
