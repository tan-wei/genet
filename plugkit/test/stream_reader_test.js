const assert = require('assert')
const {Testing, StreamReader} = require('../test')

describe('StreamReader', () => {
  describe('#addPayload()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new StreamReader()
      assert.throws(() => reader.addPayload(),   TypeError)
      assert.throws(() => reader.addPayload([]), TypeError)
    })
  })
  describe('#addSlice()', () => {
    it('should throw for wrong arguments', () => {
      const reader = new StreamReader()
      assert.throws(() => reader.addSlice(),                    TypeError)
      assert.throws(() => reader.addSlice(0),                   TypeError)
      assert.throws(() => reader.addSlice(new Uint8Array([0])), TypeError)
    })
  })
})
