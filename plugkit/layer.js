const { Attr, Payload, Layer } = require('./plugkit')

const fields = Symbol('fields')
class VirtualLayer {
  constructor (id) {
    this[fields] = {
      id,
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
    return this[fields].id
  }

  set worker (worker) {
    this[fields].worker = worker
  }

  get worker () {
    return this[fields].worker
  }

  set confidence (confidence) {
    this[fields].confidence = confidence
  }

  get confidence () {
    return this[fields].confidence
  }

  set range (range) {
    this[fields].range = range
  }

  get range () {
    return this[fields].range
  }

  get attrs () {
    return this[fields].attrs
  }

  get payloads () {
    return this[fields].payloads
  }

  get layers () {
    return this[fields].layers
  }

  get subLayers () {
    return this[fields].subLayers
  }

  get tags () {
    return this[fields].tags
  }

  get parent () {
    return this[fields].parent
  }

  get frame () {
    return this[fields].frame
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
    const { attrs } = this[fields]
    return attrs.find((attr) => attr.id === id) || null
  }

  toJSON () {
    return {}
  }
}

class WrappedLayer extends VirtualLayer {
  constructor (base, frame, parent = null) {
    super()
    this[fields] = Object.assign(this[fields], {
      base,
      frame,
      parent,
      id: base.id,
      worker: base.worker,
      confidence: base.confidence,
      attrs: base.attrs,
      layers: base.layers.map((layer) =>
        new WrappedLayer(layer, frame, this)),
      subLayers: base.subLayers.map((layer) =>
        new WrappedLayer(layer, frame, this)),
      payloads: base.payloads,
      tags: base.tags,
      range: base.range,
    })
  }
}

module.exports = {
  VirtualLayer,
  WrappedLayer,
}
