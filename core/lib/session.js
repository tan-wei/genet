import { Disposable } from 'disposables'

const fields = Symbol('fields')
export default class Session {
  constructor (config) {
    this[fields] = {
      tokens: new Map(),
      linkLayers: [],
    }
  }

  registerTokens (tokens) {
    for (const [id, data] of Object.entries(tokens)) {
      this[fields].tokens.set(id, data)
    }
    return new Disposable(() => {
      for (const id of Object.keys(tokens)) {
        this[fields].tokens.delete(id)
      }
    })
  }
}
