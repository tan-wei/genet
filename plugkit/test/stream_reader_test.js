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
      assert.throws(() => reader.search(),   TypeError)
      assert.throws(() => reader.search([]), TypeError)
    })
  })
  describe('#read()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new StreamReader()
      assert.throws(() => reader.read('aaa'), TypeError)
      assert.throws(() => reader.read(0, []), TypeError)
    })
  })
  describe('#length', () => {
    it('should return stream length', () => {
      const reader = new StreamReader()
      const array = Testing.externalize(new Uint8Array([1, 2, 3, 4, 5]))
      assert.strictEqual(0, reader.length)
      reader.addSlice(array)
      assert.strictEqual(5, reader.length)
      reader.addSlice(array)
      assert.strictEqual(10, reader.length)
    })
  })
})
