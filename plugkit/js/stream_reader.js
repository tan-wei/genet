class StreamReader {
  // Construct a new StreamReader instance
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

  /// Add all slices in `payload` to the stream.
  addPayload(payload) {
    if (typeof payload !== 'object' || payload === null ||
        payload.constructor.name !== 'Payload') {
      throw new TypeError('First argument must be an Uint8Array')
    }
    for (const slice of payload.slices) {
      this.addSlice(slice)
    }
  }

  /// Add `slice` to the stream.
  addSlice(slice) {
    if (!(slice instanceof Uint8Array)) {
      throw new TypeError('First argument must be an Uint8Array')
    }
    this._fields.length += slice.length
    this._fields.slices.push(slice)
  }

  /// Find the first occurrence of `pattern` in the stream.
  /// The first `offset` bytes in the stream are skipped.
  ///
  /// Return the _end_ position of the found byte sequence,
  /// or -1 if no such byte sequence is found.
  // @return Integer
  search(pattern, offset = 0) {
    if (!(pattern instanceof Uint8Array)) {
      throw new TypeError('First argument must be an Uint8Array')
    }
    if (!Number.isInteger(offset)) {
      throw new TypeError('Second argument must be an integer')
    }

    if (pattern.length === 0) {
      return -1
    }

    const slices = this._fields.slices
    let beginOffset = 0
    let begin = 0
    for (; begin < slices.length && (beginOffset += slices[begin].length) <= offset; ++begin);
    if (beginOffset <= offset) {
      return -1
    }
    beginOffset -= slices[begin].length
    let front = beginOffset

    for (let i = begin; i < slices.length; ++i) {
      const slice = slices[i]
      let index = 0
      if (i === begin) {
        index = offset - beginOffset
      }
      for (; index < slice.length; ++index) {
        if (slice[index] === pattern[0]) {
          const window = this.read(pattern.length, front + index)
          if (window.length === pattern.length) {
            let equal = true
            for (let j = 0; j < window.length; ++j) {
              if (window[j] !== pattern[j]) {
                equal = false
                break
              }
            }
            if (equal) {
              return front + index + pattern.length
            }
          }
        }
      }
    }

    return -1
  }

  /// Read a byte sequence from the stream up to `length` bytes.
  /// The first `offset` bytes in the stream are skipped.
  /// @return Uint8Array
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
      return new Uint8Array()
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
      data.set(slice.slice(0, data.length - dst), dst)
      dst += slice.length
    }
    return data
  }
}

exports.StreamReader = StreamReader
