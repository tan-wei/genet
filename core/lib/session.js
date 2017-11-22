import { Disposable } from 'disposables'

const fields = Symbol('fields')
export default class Session {
  constructor () {
    this[fields] = {
      tokens: new Map(),
      linkLayers: new Set(),
      dissectors: new Set(),
      nativeDissectors: new Set(),
      layerRenderers: new Set(),
      attrRenderers: new Set(),
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

  registerNativeDissector (diss) {
    this[fields].nativeDissectors.add(diss)
    return new Disposable(() => {
      this[fields].nativeDissectors.delete(diss)
    })
  }

  registerDissector (diss) {
    this[fields].dissectors.add(diss)
    return new Disposable(() => {
      this[fields].dissectors.delete(diss)
    })
  }

  registerLinkLayer (link) {
    this[fields].linkLayers.add(link)
    return new Disposable(() => {
      this[fields].linkLayers.delete(link)
    })
  }

  registerLayerRenderer (renderer) {
    this[fields].layerRenderers.add(renderer)
    return new Disposable(() => {
      this[fields].layerRenderers.delete(renderer)
    })
  }

  registerAttrRenderer (renderer) {
    this[fields].attrRenderers.add(renderer)
    return new Disposable(() => {
      this[fields].attrRenderers.delete(renderer)
    })
  }

  get tokens () {
    return this[fields].tokens.entries()
  }

  get linkLayers () {
    return this[fields].linkLayers.values()
  }

  get dissectors () {
    return this[fields].dissectors.values()
  }

  get nativeDissectors () {
    return this[fields].nativeDissectors.values()
  }

  get layerRenderers () {
    return this[fields].layerRenderers.values()
  }

  get attrRenderers () {
    return this[fields].attrRenderers.values()
  }
}
