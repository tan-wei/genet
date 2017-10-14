class StreamReader {

  // @return StreamReader
  constructor() {
    this._fields = {
      length: 0,
      slices: []
    }
  }

  // Total length of the added payloads
  // @property Integer
  get length() {
    return this._fields.length
  }

  addPayload(payload) {
    if (typeof payload !== 'object' || payload === null ||
        payload.constructor.name !== 'Payload') {
      throw new TypeError('First argument must be an Uint8Array')
    }
    for (const slice of payload.slices) {
      this.addSlice(slice)
    }
  }

  addSlice(slice) {
    if (!(slice instanceof Uint8Array)) {
      throw new TypeError('First argument must be an Uint8Array')
    }
    this._fields.length += slice.length
    this._fields.slices.push(slice)
  }

  // @return Uint8Array | null
  search(pattern, length, offset = 0) {
    if (!(pattern instanceof Uint8Array) && typeof pattern !== 'string') {
      throw new TypeError('First argument must be a string or Uint8Array')
    }
    if (!Number.isInteger(length)) {
      throw new TypeError('Second argument must be an integer')
    }
    if (!Number.isInteger(offset)) {
      throw new TypeError('Third argument must be an integer')
    }

    const slices = this._fields.slices
    let beginOffset = 0
    let begin = 0
    for (; begin < slices.length && (beginOffset += slices[begin].length) <= offset; ++begin);
    if (beginOffset <= offset) {
      return null
    }
    beginOffset -= slices[begin].length
    let front = beginOffset
  }

  read(length, offset = 0) {
    if (!Number.isInteger(length)) {
      throw new TypeError('First argument must be an integer')
    }
    if (!Number.isInteger(offset)) {
      throw new TypeError('Second argument must be an integer')
    }

    const slices = this._fields.slices
    let beginOffset = 0
    let begin = 0
    for (; begin < slices.length && (beginOffset += slices[begin].length) <= offset; ++begin);
    if (beginOffset <= offset) {
      return null
    }
    beginOffset -= slices[begin].length
    let endOffset = beginOffset
    let end = begin
    for (; end < slices.length && (endOffset += slices[end].length) < offset + length; ++end);
    let continuous = true
    const buflen = Math.min(length, endOffset - beginOffset)
    const sliceOffset = offset - beginOffset
    if (slices[begin].length >= sliceOffset + buflen) {
      return slices[begin].slice(sliceOffset, sliceOffset + buflen)
    }
    const data = new Uint8Array(buflen)
    let dst = 0
    for (let i = begin; i <= end; ++i) {
      let slice = slices[i]
      if (i === begin) {
        slice = slice.slice(offset - beginOffset)
      }
      data.set(slice, dst)
      dst += slice.length
    }
    return data
  }
}

exports.StreamReader = StreamReader
