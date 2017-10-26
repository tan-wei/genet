const layer = {}
Object.defineProperty(layer, 'ConfDecodable', { value: 0 })
Object.defineProperty(layer, 'ConfPossible', { value: 1 })
Object.defineProperty(layer, 'ConfProbable', { value: 2 })
Object.defineProperty(layer, 'ConfExact', { value: 3 })
exports.Layer = layer

class Layer {

  // Find the first child `Attr` with the given `id` and return it.
  //
  // If no attribute is found, return `null`.
  // @return Attr | null
  attr (id) {}

  // @return Layer
  addLayer (id) {}

  // @return Layer
  addSubLayer (id) {}

  // @return Paylaod
  addPayload () {}

  // @return Attr
  addAttr (id) {}

  addTag (id) {}

  // Return a JSON representation of the layer.
  // @return Object
  toJSON () {}

  // @property String
  get id () {}

  // @property Integer
  get worker () {}

  // @property Integer
  set worker (worker) {}

  // @property Integer
  get confidence () {}

  // @property Integer
  set confidence (confidence) {}

  // @property Layer | null
  get parent () {}

  // @property Frame
  get frame () {}

  // Array of the child layers.
  // @property Array<Layer>
  get layers () {}

  // Array of the layer attrs.
  // @property Array<Attr>
  get attrs () {}

  // Array of the layer payloads.
  // @property Array<Payload>
  get payloads () {}

  // Array of the layer tags.
  // @property Array<String>
  get tags () {}
}
