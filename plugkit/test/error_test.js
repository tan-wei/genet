const assert = require('assert')
const {Error, Token} = require('../test')

describe('Error', () => {
  describe('#constructor', () => {
    it('should throw for wrong arguments', () => {
      assert.throws(() => new Error()           , TypeError)
      assert.throws(() => new Error('.dst')     , TypeError)
      assert.throws(() => new Error('.dst', []) , TypeError)
      assert.throws(() => new Error([], 'Invalid value') , TypeError)
    })
  })
  describe('#target', () => {
    it('should return error target', () => {
      const error = new Error('.dst', 'Invalid value')
      assert.strictEqual('.dst', error.target)
    })
  })
  describe('#type', () => {
    it('should return error type', () => {
      const error = new Error('.dst', 'Invalid value')
      assert.strictEqual('Invalid value', error.type)
    })
  })
})
