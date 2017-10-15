const layer = {}
Object.defineProperty(layer, 'ConfDecodable', {value: 0})
Object.defineProperty(layer, 'ConfPossible',  {value: 1})
Object.defineProperty(layer, 'ConfProbable',  {value: 2})
Object.defineProperty(layer, 'ConfExact',     {value: 3})
exports.Layer = layer

class Layer {

  // @return Attr | null
  attr(id) {}

  // @return Layer
  addLayer(id) {}

  // @return Layer
  addSubLayer(id) {}

  // @return Paylaod
  addPayload() {}

  // @return Attr
  addAttr(id) {}

  addTag(id) {}

  // @return Object
  toJSON() {}

  // @property String
  get id() {}

  // @property Integer
  get worker() {}

  // @property Integer
  set worker(worker) {}

  // @property Integer
  get confidence() {}

  // @property Integer
  set confidence(confidence) {}

  // @property Layer | null
  get parent() {}

  // @property Frame
  get frame() {}

  // @property Array<Layer>
  get layers() {}

  // @property Array<Attr>
  get attrs() {}

  // @property Array<Payload>
  get payloads() {}

  // @property Array<String>
  get tags() {}
}
