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
      layerRenderers: new Set(),
      attrRenderers: new Set(),
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

  registerFilterTransform (renderer) {
    this[fields].filterTransforms.add(renderer)
    return new Disposable(() => {
      this[fields].filterTransforms.delete(renderer)
    })
  }

  async create (ifs = '') {
    const { config, tokens, linkLayers,
      dissectors, filterTransforms } = this[fields]
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
