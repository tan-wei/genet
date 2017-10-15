class Payload {

  addSlice(slice) {}

  // @return Attr
  addAttr(id) {}

  // @return Attr | null
  attr(id) {}

  // @property Array<Uint8Array>
  get slices() {}

  // @property Integer
  get length() {}

  // @property Array<Attr>
  get attrs() {}

  // @property String
  get type() {}

  // @property String
  set type(type) {}
}
