const { Attr, Payload, Layer } = require('./plugkit')

const fields = Symbol('fields')
class VirtualLayer {
  constructor (id, base = {}) {
    this[fields] = {
      id,
      base,
      worker: 0,
      confidence: Layer.ConfExact,
      attrs: [],
      layers: [],
      subLayers: [],
      payloads: [],
      tags: [],
      range: [0, 0],
      parent: null,
      frame: null,
    }
  }

  get id () {
    const { id, base } = this[fields]
    return base.id || id
  }

  set worker (worker) {
    this[fields].worker = worker
  }

  get worker () {
    const { worker, base } = this[fields]
    return base.worker || worker
  }

  set confidence (confidence) {
    this[fields].confidence = confidence
  }

  get confidence () {
    const { confidence, base } = this[fields]
    return base.confidence || confidence
  }

  set range (range) {
    this[fields].range = range
  }

  get range () {
    const { range, base } = this[fields]
    return base.range || range
  }

  get attrs () {
    const { attrs, base } = this[fields]
    return (base.attrs || []).concat(attrs)
  }

  get payloads () {
    const { payloads, base } = this[fields]
    return (base.payloads || []).concat(payloads)
  }

  get layers () {
    const { layers, base } = this[fields]
    return (base.layers || []).concat(layers)
  }

  get subLayers () {
    const { subLayers, base } = this[fields]
    return (base.subLayers || []).concat(subLayers)
  }

  get tags () {
    const { tags, base } = this[fields]
    return (base.tags || []).concat(tags)
  }

  get parent () {
    const { parent, base } = this[fields]
    return base.parent || parent
  }

  get frame () {
    const { frame, base } = this[fields]
    return base.frame || frame
  }

  addTag (tag) {
    const { tags } = this[fields]
    tags.push(tag)
  }

  addAttr (id) {
    const { attrs } = this[fields]
    const attr = Attr.create(id)
    attrs.push(attr)
    return attr
  }

  addLayer (id) {
    const { layers } = this[fields]
    const layer = new VirtualLayer(id)
    layer[fields].frame = this[fields].frame
    layer[fields].parent = this
    layers.push(layer)
    return layer
  }

  addSubLayer (id) {
    const { subLayers } = this[fields]
    const layer = new VirtualLayer(id)
    layer[fields].frame = this[fields].frame
    layer[fields].parent = this
    subLayers.push(layer)
    return layer
  }

  addPayload () {
    const { payloads } = this[fields]
    const payload = Payload.create()
    payloads.push(payload)
    return payload
  }

  attr (id) {
    const { attrs, base } = this[fields]
    if (base.attr) {
      const result = base.attr(id)
      if (result !== null) {
        return result
      }
    }
    return attrs.find((attr) => attr.id === id) || null
  }

  toJSON () {
    return {}
  }
}

module.exports = VirtualLayer

/*
Static NAN_METHOD(attr);

*/
