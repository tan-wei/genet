const assert = require('assert')
const {Testing, StreamReader} = require('../test')

describe('StreamReader', () => {
  describe('#addPayload()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new StreamReader()
      assert.throws(() => reader.addPayload(),   TypeError)
      assert.throws(() => reader.addPayload([]), TypeError)
    })
    it('should accept a Payload', () => {
      const reader = new StreamReader()
      const array = Testing.externalize(new Uint8Array([0]))
      const payload = Testing.createPayloadInstance()
      payload.addSlice(array)
      assert.doesNotThrow(() => reader.addPayload(payload))
    })
  })
  describe('#addSlice()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new StreamReader()
      assert.throws(() => reader.addSlice(),  TypeError)
      assert.throws(() => reader.addSlice(0), TypeError)
    })
    it('should accept an Uint8Array', () => {
      const reader = new StreamReader()
      assert.doesNotThrow(() => reader.addSlice(new Uint8Array([0])))
    })
  })
  describe('#search()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new StreamReader()
      assert.throws(() => reader.search(),                     TypeError)
      assert.throws(() => reader.search([]),                   TypeError)
      assert.throws(() => reader.search(new Uint8Array(), []), TypeError)
      assert.throws(() => reader.search([], 0),                TypeError)
    })
    it('should return -1 when no such pattern is found', () => {
      const reader = new StreamReader()
      reader.addSlice(new Uint8Array([1, 2, 3, 4, 5]))
      assert.strictEqual(-1, reader.search(new Uint8Array([3, 2])))
    })
    it('should return -1 for an empty pattern', () => {
      const reader = new StreamReader()
      reader.addSlice(new Uint8Array([1, 2, 3, 4, 5]))
      assert.strictEqual(-1, reader.search(new Uint8Array()))
    })
    it('should return the end position of the found pattern', () => {
      const reader = new StreamReader()
      reader.addSlice(new Uint8Array([1, 2, 3]))
      reader.addSlice(new Uint8Array([4, 5, 6]))
      reader.addSlice(new Uint8Array([7, 8, 9]))
      assert.strictEqual(7, reader.search(new Uint8Array([2, 3, 4, 5, 6, 7])))
    })
  })
  describe('#read()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new StreamReader()
      assert.throws(() => reader.read('aaa'), TypeError)
      assert.throws(() => reader.read(0, []), TypeError)
    })
    it('should return an empty Uint8Array when no slices are added', () => {
      const reader = new StreamReader()
      assert.strictEqual(0, reader.read(5).length)
    })
    it('should return a sliced stream', () => {
      const reader = new StreamReader()
      reader.addSlice(new Uint8Array([1, 2, 3, 4, 5]))
      assert.strictEqual('1,2,3,4', reader.read(4).toString())
    })
  })
  describe('#length', () => {
    it('should return stream length', () => {
      const reader = new StreamReader()
      const array = new Uint8Array([1, 2, 3, 4, 5])
      assert.strictEqual(0, reader.length)
      reader.addSlice(array)
      assert.strictEqual(5, reader.length)
      reader.addSlice(array)
      assert.strictEqual(10, reader.length)
    })
  })
})
