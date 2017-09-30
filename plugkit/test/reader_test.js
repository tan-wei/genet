const assert = require('assert')
const {Testing, Error, Reader} = require('../test')

describe('Reader', () => {
  describe('#sliceAll()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getUint16([]), TypeError)
    })
  })
  describe('#getUint8()', () => {
    it('should return an Uint8 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([200, 250, -1, -2]))
      assert.strictEqual(200, reader.getUint8())
      assert.strictEqual(250, reader.getUint8())
      assert.strictEqual(255, reader.getUint8())
      assert.strictEqual(254, reader.getUint8())
      assert.strictEqual(0, reader.getUint8())
    })
  })
  describe('#getInt8()', () => {
    it('should return an Int8 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([200, 250, -1, -2]))
      assert.strictEqual(-56, reader.getInt8())
      assert.strictEqual(-6, reader.getInt8())
      assert.strictEqual(-1, reader.getInt8())
      assert.strictEqual(-2, reader.getInt8())
      assert.strictEqual(0, reader.getInt8())
    })
  })
  describe('#getUint16()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getUint16([]), TypeError)
    })
    it('should return an Uint16 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(51450, reader.getUint16())
      assert.strictEqual(65534, reader.getUint16())
      assert.strictEqual(0, reader.getUint16())
    })
  })
  describe('#getUint32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getUint32([]), TypeError)
    })
    it('should return an Uint32 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(3371892734, reader.getUint32())
      assert.strictEqual(0, reader.getUint32())
    })
  })
  describe('#getInt16()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getInt16([]), TypeError)
    })
    it('should return an Int16 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(-14086, reader.getInt16())
      assert.strictEqual(-2, reader.getInt16())
      assert.strictEqual(0, reader.getInt16())
    })
  })
  describe('#getInt32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getInt32([]), TypeError)
    })
    it('should return an Int32 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(-923074562, reader.getInt32())
      assert.strictEqual(0, reader.getInt32())
    })
  })
  describe('#getFloat32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getFloat32([]), TypeError)
    })
    it('should return a Float32 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([200, 250, -1, -2, 0]))
      assert.strictEqual(-514047.9375, reader.getFloat32())
      assert.strictEqual(0.0, reader.getFloat32())
    })
  })
  describe('#getFloat64()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getFloat64([]), TypeError)
    })
    it('should return a Float64 value', () => {
      const reader = new Reader()
      reader.data = Testing.externalize(new Uint8Array([-64, 0, 0, 0, 0, 0, 0, 0, 0]))
      assert.strictEqual(-2.0, reader.getFloat64())
      assert.strictEqual(0.0, reader.getFloat64())
    })
  })
  describe('#data', () => {
    it('should return reader data', () => {
      const reader = new Reader()
      const array = Testing.externalize(new Uint8Array([1, 2, 3, 4, 5]))
      assert.strictEqual(0, reader.data.length)
      reader.data = array
      assert.strictEqual(5, reader.data.length)
    })
  })
  describe('#lastRange', () => {
    it('should return reader last range', () => {
      const reader = new Reader()
      assert.deepEqual([0, 0], reader.lastRange)
      reader.lastRange = [123, 456]
      assert.deepEqual([123, 456], reader.lastRange)
    })
  })
  describe('#lastError', () => {
    it('should return reader last error', () => {
      const reader = new Reader()
      assert.deepEqual('Error', reader.lastError.constructor.name)
      reader.lastError = new Error('.dst', 'Invalid value')
      assert.strictEqual('.dst', reader.lastError.target)
      assert.strictEqual('Invalid value', reader.lastError.type)
    })
  })
})
