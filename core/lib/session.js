import { Disposable } from 'disposables'
import { SessionFactory } from 'plugkit'

const fields = Symbol('fields')
export default class Session {
  constructor (config) {
    this[fields] = {
      config,
      tokens: new Map(),
      linkLayers: new Set(),
      dissectors: new Set(),
      layerRenderers: new Map(),
      attrRenderers: new Map(),
      filterTransforms: new Set(),
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

  registerLayerRenderer (id, renderer) {
    this[fields].layerRenderers.set(id, renderer)
    return new Disposable(() => {
      this[fields].layerRenderers.delete(id)
    })
  }

  registerAttrRenderer (id, renderer) {
    this[fields].attrRenderers.set(id, renderer)
    return new Disposable(() => {
      this[fields].attrRenderers.delete(id)
    })
  }

  registerFilterTransform (renderer) {
    this[fields].filterTransforms.add(renderer)
    return new Disposable(() => {
      this[fields].filterTransforms.delete(renderer)
    })
  }

  token (id) {
    const data = this[fields].tokens.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return { name: id }
  }

  layerRenderer (id) {
    const data = this[fields].layerRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  attrRenderer (id) {
    const data = this[fields].attrRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  async create (ifs = '') {
    const {
      config, tokens, linkLayers,
      dissectors, filterTransforms,
    } = this[fields]
    const factory = new SessionFactory()
    factory.options = config.toJSON()
    factory.networkInterface = ifs
    for (const layer of linkLayers) {
      factory.registerLinkLayer(layer)
    }
    for (const diss of dissectors) {
      factory.registerDissector(diss)
    }
    for (const trans of filterTransforms) {
      factory.registerFilterTransform(trans)
    }
    const attributes = {}
    for (const [key, value] of tokens.entries()) {
      attributes[key] = value
    }
    factory.registerAttributes(attributes)
    return factory.create()
  }
}
