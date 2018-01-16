const fields = Symbol('fields')
class Reader {
  // Construct a new Reader instance
  // @return Reader
  constructor (data) {
    if (!(data instanceof Uint8Array)) {
      throw new TypeError('First argument must be an Uint8Array')
    }
    this[fields] = {
      data,
      lastRange: [0, 0],
      lastError: '',
      getDateView: () =>
        new DataView(data.buffer, data.byteOffset, data.byteLength),
      nextRange: (size) => {
        [, this[fields].lastRange[0]] = this[fields].lastRange
        this[fields].lastRange[1] = this[fields].lastRange[0] + size
      },
    }
  }

  // @return UInt8Array
  slice (begin, end) {
    if (!Number.isInteger(begin)) {
      throw new TypeError('First argument must be an integer')
    }
    if (!Number.isInteger(end)) {
      throw new TypeError('Second argument must be an integer')
    }
    if (end < begin) {
      throw new
      RangeError('Second argument must be greater than first argument')
    }
    const [, offset] = this[fields].lastRange
    const { length } = this[fields].data
    const sliceBegin = offset + begin
    const sliceEnd = offset + end
    if (sliceBegin >= length || sliceEnd > length) {
      this[fields].lastError = '!out-of-bounds'
      return this[fields].data.slice(0, 0)
    }
    const slice = this[fields].data.slice(sliceBegin, sliceEnd);
    [, this[fields].lastRange[0]] = this[fields].lastRange
    this[fields].lastRange[1] = sliceEnd
    return slice
  }

  // @return UInt8Array
  sliceAll (begin = 0) {
    if (!Number.isInteger(begin)) {
      throw new TypeError('First argument must be an integer')
    }
    const [, offset] = this[fields].lastRange
    const { length } = this[fields].data
    const sliceBegin = offset + begin
    if (sliceBegin >= length) {
      this[fields].lastError = '!out-of-bounds'
      return this[fields].data.slice(0, 0)
    }
    const slice = this[fields].data.slice(sliceBegin);
    [, this[fields].lastRange[0]] = this[fields].lastRange
    this[fields].lastRange[1] = length
    return slice
  }

  // Returns an unsigned 8-bit integer
  // @return Integer
  getUint8 () {
    try {
      const value =
        this[fields].getDateView().getUint8(this[fields].lastRange[1])
      this[fields].nextRange(1)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @return Integer
  getInt8 () {
    try {
      const value =
        this[fields].getDateView().getInt8(this[fields].lastRange[1])
      this[fields].nextRange(1)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @return Integer
  getUint16 (littleEndian = false) {
    if (typeof littleEndian !== 'boolean') {
      throw new TypeError('First argument must be boolean')
    }
    try {
      const value =
        this[fields].getDateView().getUint16(
          this[fields].lastRange[1], littleEndian)
      this[fields].nextRange(2)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @return Integer
  getUint32 (littleEndian = false) {
    if (typeof littleEndian !== 'boolean') {
      throw new TypeError('First argument must be boolean')
    }
    try {
      const value = this[fields]
        .getDateView()
        .getUint32(this[fields].lastRange[1], littleEndian)
      this[fields].nextRange(4)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @return Integer
  getInt16 (littleEndian = false) {
    if (typeof littleEndian !== 'boolean') {
      throw new TypeError('First argument must be boolean')
    }
    try {
      const value = this[fields]
        .getDateView()
        .getInt16(this[fields].lastRange[1], littleEndian)
      this[fields].nextRange(2)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @return Integer
  getInt32 (littleEndian = false) {
    if (typeof littleEndian !== 'boolean') {
      throw new TypeError('First argument must be boolean')
    }
    try {
      const value = this[fields]
        .getDateView()
        .getInt32(this[fields].lastRange[1], littleEndian)
      this[fields].nextRange(4)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @return Double
  getFloat32 (littleEndian = false) {
    if (typeof littleEndian !== 'boolean') {
      throw new TypeError('First argument must be boolean')
    }
    try {
      const value = this[fields]
        .getDateView()
        .getFloat32(this[fields].lastRange[1], littleEndian)
      this[fields].nextRange(4)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @return Double
  getFloat64 (littleEndian = false) {
    if (typeof littleEndian !== 'boolean') {
      throw new TypeError('First argument must be boolean')
    }
    try {
      const value = this[fields]
        .getDateView()
        .getFloat64(this[fields].lastRange[1], littleEndian)
      this[fields].nextRange(8)
      return value
    } catch (err) {
      this[fields].lastError = '!out-of-bounds'
      return 0
    }
  }

  // @property [Integer, Integer]
  get lastRange () {
    return this[fields].lastRange
  }

  // @property [Integer, Integer]
  set lastRange (value) {
    if (Array.isArray(value) && value.length >= 2) {
      this[fields].lastRange = value.slice(0, 2)
    }
  }

  // @property String
  get lastError () {
    return this[fields].lastError
  }

  // @property Uint8Array
  get data () {
    return this[fields].data
  }
}

exports.Reader = Reader
