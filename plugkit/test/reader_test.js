const assert = require('assert')
const {Testing, Reader} = require('../test')

describe('Reader', () => {
  describe('#data', () => {
    it('should return reader data', () => {
      const reader = new Reader()
      const array = Testing.externalize(new Uint8Array([1, 2, 3, 4, 5]))
      assert.equal(0, reader.data.length)
      reader.data = array
      assert.equal(5, reader.data.length)
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
    })
  })
})
