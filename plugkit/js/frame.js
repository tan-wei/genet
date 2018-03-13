class Frame {
  // @property Date
  get timestamp () {}

  // @property Integer
  get length () {}

  // @property Integer
  get index () {}

  // @property Layer
  get rootLayer () {}

  // @property Layer
  get primaryLayer () {}

  // @property Array<Layer>
  get leafLayers () {}

  // @return Attr | null
  attr (id) {}

  // @return Layer | null
  layer (id) {}
}
