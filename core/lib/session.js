const fields = Symbol('fields')
export default class Session {
  constructor (config) {
    this[fields] = { tokens: new Map() }
  }

  registerTokens (tokens) {

  }

  unregisterTokens (tokens) {

  }
}
