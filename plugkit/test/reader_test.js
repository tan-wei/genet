const assert = require('assert')
const {Error, Reader} = require('../test')

describe('Reader', () => {
  describe('#slice()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.slice(),      TypeError)
      assert.throws(() => reader.slice(0),     TypeError)
      assert.throws(() => reader.slice(0, []), TypeError)
      assert.throws(() => reader.slice(5, 0),  RangeError)
    })
  })
  describe('#sliceAll()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.sliceAll([]), TypeError)
    })
  })
  describe('#getUint8()', () => {
    it('should return an Uint8 value', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2]))
      assert.strictEqual(200, reader.getUint8())
      assert.strictEqual(250, reader.getUint8())
      assert.strictEqual(255, reader.getUint8())
      assert.strictEqual(254, reader.getUint8())
      assert.strictEqual(0, reader.getUint8())
    })
  })
  describe('#getInt8()', () => {
    it('should return an Int8 value', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2]))
      assert.strictEqual(-56, reader.getInt8())
      assert.strictEqual(-6, reader.getInt8())
      assert.strictEqual(-1, reader.getInt8())
      assert.strictEqual(-2, reader.getInt8())
      assert.strictEqual(0, reader.getInt8())
    })
  })
  describe('#getUint16()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.getUint16([]), TypeError)
    })
    it('should return a big-endian Uint16 value', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(51450, reader.getUint16())
      assert.strictEqual(65534, reader.getUint16())
      assert.strictEqual(0, reader.getUint16())
    })
    it('should return a little-endian Uint16 value when the first argument is true', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(64200, reader.getUint16(true))
      assert.strictEqual(65279, reader.getUint16(true))
      assert.strictEqual(0, reader.getUint16(true))
    })
  })
  describe('#getUint32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.getUint32([]), TypeError)
    })
    it('should return a big-endian Uint32 value', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(3371892734, reader.getUint32())
      assert.strictEqual(0, reader.getUint32())
    })
    it('should return a little-endian Uint32 value when the first argument is true', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(4278188744, reader.getUint32(true))
      assert.strictEqual(0, reader.getUint32(true))
    })
  })
  describe('#getInt16()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.getInt16([]), TypeError)
    })
    it('should return a big-endian Int16 value', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(-14086, reader.getInt16())
      assert.strictEqual(-2, reader.getInt16())
      assert.strictEqual(0, reader.getInt16())
    })
    it('should return a little-endian Int16 value when the first argument is true', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(-1336, reader.getInt16(true))
      assert.strictEqual(-257, reader.getInt16(true))
      assert.strictEqual(0, reader.getInt16(true))
    })
  })
  describe('#getInt32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.getInt32([]), TypeError)
    })
    it('should return a big-endian Int32 value', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(-923074562, reader.getInt32())
      assert.strictEqual(0, reader.getInt32())
    })
    it('should return a little-endian Int32 value when the first argument is true', () => {
      const reader = new Reader(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(-16778552, reader.getInt32(true))
      assert.strictEqual(0, reader.getInt32(true))
    })
  })
  describe('#getFloat32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.getFloat32([]), TypeError)
    })
    it('should return a big-endian Float32 value', () => {
      const reader = new Reader(new Uint8Array([-64, 0, 0, 0, 0]))
      assert.strictEqual(-2, reader.getFloat32())
      assert.strictEqual(0.0, reader.getFloat32())
    })
    it('should return a little-endian Float32 value when the first argument is true', () => {
      const reader = new Reader(new Uint8Array([0, 0, 0, -64, 0]))
      assert.strictEqual(-2, reader.getFloat32(true))
      assert.strictEqual(0.0, reader.getFloat32(true))
    })
  })
  describe('#getFloat64()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader(new Uint8Array())
      assert.throws(() => reader.getFloat64([]), TypeError)
    })
    it('should return a big-endian Float64 value', () => {
      const reader = new Reader(new Uint8Array([-64, 0, 0, 0, 0, 0, 0, 0, 0]))
      assert.strictEqual(-2.0, reader.getFloat64())
      assert.strictEqual(0.0, reader.getFloat64())
    })
    it('should return a little-endian Float64 value when the first argument is true', () => {
      const reader = new Reader(new Uint8Array([0, 0, 0, 0, 0, 0, 0, -64, 0]))
      assert.strictEqual(-2, reader.getFloat64(true))
      assert.strictEqual(0.0, reader.getFloat64(true))
    })
  })
  describe('#data', () => {
    it('should return reader data', () => {
      const reader = new Reader(new Uint8Array([1, 2, 3, 4, 5]))
      assert.strictEqual(5, reader.data.length)
    })
  })
  describe('#lastRange', () => {
    it('should return reader last range', () => {
      const reader = new Reader(new Uint8Array())
      assert.deepEqual([0, 0], reader.lastRange)
      reader.lastRange = [123, 456]
      assert.deepEqual([123, 456], reader.lastRange)
    })
  })
  describe('#lastError', () => {
    it('should return reader last error', () => {
      const reader = new Reader(new Uint8Array())
      reader.getInt16()
      assert.strictEqual('!out-of-bounds', reader.lastError)
    })
  })
})
