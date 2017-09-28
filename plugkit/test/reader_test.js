const assert = require('assert')
const {Testing, Error, Reader} = require('../test')

describe('Reader', () => {
  describe('#getUint8()', () => {
    it('should return a Uint8 value', () => {
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
  })
  describe('#getUint32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getUint32([]), TypeError)
    })
  })
  describe('#getInt16()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getInt16([]), TypeError)
    })
  })
  describe('#getInt32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getInt32([]), TypeError)
    })
  })
  describe('#getFloat32()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getFloat32([]), TypeError)
    })
  })
  describe('#getFloat64()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new Reader()
      assert.throws(() => reader.getFloat64([]), TypeError)
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
