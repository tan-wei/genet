class StreamReader {
  constructor() {
    this._fields = {
      length: 0,
      slices: []
    }
  }

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
    const buflen = endOffset - beginOffset
    const sliceOffset = offset - beginOffset
    if (slices[begin].length >= sliceOffset + buflen) {
      return slices[begin].slice(sliceOffset, sliceOffset + buflen)
    }
  }
}

exports.StreamReader = StreamReader
