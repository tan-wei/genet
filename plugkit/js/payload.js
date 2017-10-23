class Payload {

  addSlice(slice) {}

  addAttr(id) {}

  // Find the first child `Attr` with the given `id` and return it.
  //
  // If no attribute is found, return `null`.
  // @return Attr | null
  attr(id) {}

  // @property Array<Uint8Array>
  // Array of the slices.
  get slices() {}

  // @property Integer
  // Length of the payload.
  get length() {}

  // @property Array<Attr>
  // Array of the payload attrs.
  get attrs() {}

  // @property String
  // Type of the payload.
  get type() {}

  // @property String
  // Type of the payload.
  set type(type) {}
}
